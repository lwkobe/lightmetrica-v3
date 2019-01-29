/*
    Lightmetrica - Copyright (c) 2019 Hisanari Otsu
    Distributed under MIT license. See LICENSE file for details.
*/

#include <pch.h>
#include <lm/material.h>
#include <lm/surface.h>

LM_NAMESPACE_BEGIN(LM_NAMESPACE)

/*
\rst
.. function:: material::mask
   
   Pass-through material.

   This component implements a special material that only sample
   the outgoing ray into the same direction as the incoming ray.
   This material is used to implement texture-masked materials.
   BSDF reads

   .. math::
      f_s(\omega_i, \omega_o) = \delta_\Omega(-\omega_i, \omega_o).
\endrst
*/
class Material_Mask final : public Material {
private:
    virtual bool isSpecular(const PointGeometry&) const override {
        return true;
    }

    virtual std::optional<MaterialDirectionSample> sample(Rng&, const PointGeometry&, Vec3 wi) const override {
        return MaterialDirectionSample{
            -wi,
            0,
            Vec3(1_f)
        };
    }
};

LM_COMP_REG_IMPL(Material_Mask, "material::mask");

LM_NAMESPACE_END(LM_NAMESPACE)
