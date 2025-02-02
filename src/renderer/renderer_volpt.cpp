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

#define VOLPT_DEBUG_VIS 0

LM_NAMESPACE_BEGIN(LM_NAMESPACE)

class Renderer_VolPT final : public Renderer {
private:
    Film* film_;
    long long spp_;
    int maxLength_;

    #if VOLPT_DEBUG_VIS
    mutable std::vector<Ray> sampledRays_;
    #endif

public:
    LM_SERIALIZE_IMPL(ar) {
        ar(film_, spp_, maxLength_);
        #if VOLPT_DEBUG_VIS
        ar(sampledRays_);
        #endif
    }

    virtual void foreachUnderlying(const ComponentVisitor& visit) override {
        comp::visit(visit, film_);
    }

    #if VOLPT_DEBUG_VIS
    virtual void* underlyingRawPointer(const std::string& query) const override {
        if (query == "sampledRays") {
            return &sampledRays_;
        }
        return nullptr;
    }
    #endif

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
        long long numSamples = (long long)(w*h)*spp_;
        parallel::foreach(numSamples, [&](long long index, int threadId) -> void {
            LM_UNUSED(threadId);

            // Per-thread random number generator
            thread_local Rng rng;

            // Pixel positions
            const auto j = index / spp_;
            const int x = int(j % w);
            const int y = int(j / w);
            
            // Estimate pixel contribution
            Vec3 L(0_f);

            // Incident ray direction
            Vec3 wi;

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

                // Sample a NEE edge
                const bool nee = length > 0 && !scene->isSpecular(s->sp);
                if (nee) [&] {
                    // Sample a light
                    const auto sL = scene->sampleLight(rng, s->sp);
                    if (!sL) {
                        return;
                    }
                    
                    // Transmittance
                    const auto Tr = scene->evalTransmittance(rng, s->sp, sL->sp);
                    if (!Tr) {
                        return;
                    }

                    #if VOLPT_DEBUG_VIS
                    const bool record = 500 < x && x < 600 && 500 < y && y < 600;
                    if (threadId == 0 && sampledRays_.size() < 1000 && record) {
                        sampledRays_.push_back({ s->sp.geom.p, -sL->wo });
                    }
                    #endif

                    // Evaluate and accumulate contribution
                    const auto wo = -sL->wo;
                    const auto fs = scene->evalContrb(s->sp, wi, wo);
                    L += throughput * *Tr * fs * sL->weight;
                }();

                // Sample next scene interaction
                const auto sd = scene->sampleDistance(rng, s->sp, s->wo);
                if (!sd) {
                    break;
                }

                // Update throughput
                throughput *= s->weight * sd->weight;

                // Accumulate contribution from emissive interaction
                if (!nee && scene->isLight(sd->sp)) {
                    L += throughput * scene->evalContrbEndpoint(sd->sp, -s->wo);
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
                wi = -s->wo;
                sampleRay = [&, sp = sd->sp]() {
                    return scene->sampleRay(rng, sp, wi);
                };
            }

            // Set color of the pixel
            film_->splatPixel(x, y, L / Float(spp_));
        });
    }
};

LM_COMP_REG_IMPL(Renderer_VolPT, "renderer::volpt");

LM_NAMESPACE_END(LM_NAMESPACE)
