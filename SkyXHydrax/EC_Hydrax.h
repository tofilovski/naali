/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Hydrax.h
 *  @brief  A photorealistic water plane component using Hydrax, http://www.ogre3d.org/tikiwiki/Hydrax
 */

#pragma once

#include "IComponent.h"
#include "Math/float3.h"

struct EC_HydraxImpl;

/// A photorealistic water plane component using Hydrax, http://www.ogre3d.org/tikiwiki/Hydrax
/** Provides means of creating photorealistic environments together with EC_SkyX.
    @note Requires Hydrax Ogre add-on. */
class EC_Hydrax : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Hydrax", 777)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Hydrax(Scene* scene);
    ~EC_Hydrax();

    /// Is the water visible.
    DEFINE_QPROPERTY_ATTRIBUTE(bool, visible);
    Q_PROPERTY(bool visible READ getvisible WRITE setvisible);

    /// Position of the water.
    DEFINE_QPROPERTY_ATTRIBUTE(float3, position);
    Q_PROPERTY(float3 position READ getposition WRITE setposition);

private:
    EC_HydraxImpl *impl;

private slots:
    void UpdateAttribute(IAttribute *attr);
    void Update(float frameTime);
};