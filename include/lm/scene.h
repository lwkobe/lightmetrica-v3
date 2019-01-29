/*
    Lightmetrica - Copyright (c) 2019 Hisanari Otsu
    Distributed under MIT license. See LICENSE file for details.
*/

#pragma once

#include "component.h"
#include "math.h"
#include "surface.h"
#include <variant>

LM_NAMESPACE_BEGIN(LM_NAMESPACE)

// ----------------------------------------------------------------------------

/*!
    \addtogroup scene
    @{
*/

/*!
    \brief Result of sampleRay functions.
*/
struct RaySample {
    SurfacePoint sp;   // Surface point information
    Vec3 wo;           // Sampled direction
    Vec3 weight;       // Contribution divided by probability

    // Get a ray from the sample
    Ray ray() const {
        assert(!sp.geom.infinite);
        return { sp.geom.p, wo };
    }
};

// ----------------------------------------------------------------------------

/*!
    \brief Scene.
*/
class Scene : public Component {
public:
    /*!
        \brief Load a scene primitive.
    */
    virtual bool loadPrimitive(
        const Component& assetGroup, Mat4 transform, const Json& prop) = 0;

    /*!
        \brief Create primitives from a model.
    */
    virtual bool loadPrimitives(
        const Component& assetGroup, Mat4 transform, const std::string& modelName) = 0;

    /*!
        \brief Iterate triangles in the scene.
    */
    using ProcessTriangleFunc = std::function<
        void(int primitive, int face, Vec3 p1, Vec3 p2, Vec3 p3)>;
    virtual void foreachTriangle(
        const ProcessTriangleFunc& processTriangle) const = 0;

    // ------------------------------------------------------------------------

    /*!
        \brief Build acceleration structure.
        \param name Name of the acceleration structure.
        \param prop Property for configuration.
    */
    virtual void build(const std::string& name, const Json& prop) = 0;

    /*!
        \brief Compute closest intersection point.
    */
    virtual std::optional<SurfacePoint> intersect(
        Ray ray, Float tmin = Eps, Float tmax = Inf) const = 0;

    /*!
        \brief Check if two surface points are mutually visible.
    */
    bool visible(const SurfacePoint& sp1, const SurfacePoint& sp2) const {
        const auto visible_ = [this](const SurfacePoint& sp1, const SurfacePoint& sp2) -> bool {
            assert(!sp1.geom.infinite);
            const auto wo = sp2.geom.infinite
                ? -sp2.geom.wo
                : glm::normalize(sp2.geom.p - sp1.geom.p);
            const auto tmax = sp2.geom.infinite
                ? Inf
                : [&]() {
                    const auto d = glm::distance(sp1.geom.p, sp2.geom.p);
                    return d * (1_f - Eps);
                }();
            return !intersect(Ray{sp1.geom.p, wo}, Eps, tmax);
        };
        if (sp1.geom.infinite) {
            return visible_(sp2, sp1);
        }
        else {
            return visible_(sp1, sp2);
        }
    }

    // ------------------------------------------------------------------------

    /*!
        \brief Check if given surface point is light.
    */
    virtual bool isLight(const SurfacePoint& sp) const = 0;

    /*!
        \brief Check if given surface point is specular.
    */
    virtual bool isSpecular(const SurfacePoint& sp) const = 0;

    // ------------------------------------------------------------------------

    /*!
        \brief Generate a primary ray.
    */
    virtual Ray primaryRay(Vec2 rp) const = 0;

    /*!
        \brief Sample a ray given surface point and incident direction.
        \rst
        (x,wo) ~ p(x,wo|sp,wi)
        \endrst
    */
    virtual std::optional<RaySample> sampleRay(Rng& rng, const SurfacePoint& sp, Vec3 wi) const = 0;

    /*!
        \brief Sample a ray given pixel position.
        \rst
        (x,wo) ~ p(x,wo|raster window)
        \endrst
    */
    virtual std::optional<RaySample> samplePrimaryRay(Rng& rng, Vec4 window) const = 0;

    /*!
        \brief Sample a position on a light.
    */
    virtual std::optional<RaySample> sampleLight(Rng& rng, const SurfacePoint& sp) const = 0;

    // ------------------------------------------------------------------------

    /*!
        \brief Evaluate extended BSDF.
    */
    virtual Vec3 evalBsdf(const SurfacePoint& sp, Vec3 wi, Vec3 wo) const = 0;

    /*!
        \brief Evaluate endpoint contribution.
        \rst
        f(x,wo) where x is endpoint
        \endrst
    */
    virtual Vec3 evalContrbEndpoint(const SurfacePoint& sp, Vec3 wo) const = 0;

    /*!
        \brief Evaluate reflectance (if available).
        \rst
        \rho(x)
        \endrst
    */
    virtual std::optional<Vec3> reflectance(const SurfacePoint& sp) const = 0;
};

/*!
    @}
*/

// ----------------------------------------------------------------------------

LM_NAMESPACE_END(LM_NAMESPACE)
