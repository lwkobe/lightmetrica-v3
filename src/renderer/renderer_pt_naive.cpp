/*
    Lightmetrica - Copyright (c) 2019 Hisanari Otsu
    Distributed under MIT license. See LICENSE file for details.
*/

#include <pch.h>
#include <lm/user.h>
#include <lm/renderer.h>
#include <lm/scene.h>
#include <lm/film.h>
#include <lm/parallel.h>
#include <lm/serial.h>

LM_NAMESPACE_BEGIN(LM_NAMESPACE)

class Renderer_PTNaive final : public Renderer {
private:
    Film* film_;
    long long spp_;
    int maxLength_;

public:
    LM_SERIALIZE_IMPL(ar) {
        ar(film_, spp_, maxLength_);
    }

    virtual void foreachUnderlying(const ComponentVisitor& visit) override {
        comp::visit(visit, film_);
    }

public:
    virtual bool construct(const Json& prop) override {
        film_ = comp::get<Film>(prop["output"]);
        if (!film_) {
            return false;
        }
        spp_ = prop["spp"];
        maxLength_ = prop["maxLength"];
        return true;
    }

    virtual void render(const Scene* scene) const override {
        film_->clear();
        const auto [w, h] = film_->size();
        parallel::foreach(w*h, [&](long long index, int) -> void {
            // Per-thread random number generator
            thread_local Rng rng;

            // Pixel positions
            const int x = int(index % w);
            const int y = int(index / w);

            // Estimate pixel contribution
            Vec3 L(0_f);
            for (long long i = 0; i < spp_; i++) {
                // Path throughput
                Vec3 throughput(1_f);

                // Initial sampleRay function
                std::function<std::optional<RaySample>()> sampleRay = [&]() {
                    Float dx = 1_f/w, dy = 1_f/h;
                    return scene->samplePrimaryRay(rng, {dx*x, dy*y, dx, dy}, film_->aspectRatio());
                };

                // Perform random walk
                for (int length = 0; length < maxLength_; length++) {
                    // Sample a ray
                    const auto s = sampleRay();
                    if (!s || math::isZero(s->weight)) {
                        break;
                    }

                    // Update throughput
                    throughput *= s->weight;

                    // Intersection to next surface
                    const auto hit = scene->intersect(s->ray());
                    if (!hit) {
                        break;
                    }

                    // Accumulate contribution from light
                    if (scene->isLight(*hit)) {
                        L += throughput * scene->evalContrbEndpoint(*hit, -s->wo);
                    }

                    // Russian roulette
                    if (length > 3) {
                        const auto q = glm::max(.2_f, 1_f - glm::compMax(throughput));
                        if (rng.u() < q) {
                            break;
                        }
                        throughput /= 1_f - q;
                    }

                    // Update
                    sampleRay = [&, wi = -s->wo, sp = *hit]() {
                        return scene->sampleRay(rng, sp, wi);
                    };
                }
            }
            L /= spp_;

            // Set color of the pixel
            film_->setPixel(x, y, L);
        });
    }
};

LM_COMP_REG_IMPL(Renderer_PTNaive, "renderer::pt_naive");

LM_NAMESPACE_END(LM_NAMESPACE)
