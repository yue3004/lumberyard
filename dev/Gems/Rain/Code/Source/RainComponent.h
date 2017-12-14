/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzFramework/Components/CameraBus.h>

#include "RainComponentBus.h"

namespace Rain
{
    class RainOptions
    {
    public:
        friend class RainConverter;

        AZ_TYPE_INFO(RainOptions, "{44EEA846-4252-405F-BB66-0B8696DF5D36}");

        bool m_ignoreVisAreas = false; ///< Ignore vis area volumes
        bool m_disableOcclusion = false; ///< Ignore occlusion volumes
        float m_radius = 10000.0f; ///< Radius of the rain volume (Range: 0 to 10,000; Step: 1)
        float m_amount = 1.0f; ///< Overall amount of rain (Range: 0 to 100; Step: 0.1)
        float m_diffuseDarkening = 0.5f; ///< Amount of darkening applied by the wetness of the rain (Range: 0 to 1; Step: 0.1)
        float m_rainDropsAmount = 0.5f; ///< Amount of rain drops that can be seen in the air (Range: 0 to 100; Step: 0.1)
        float m_rainDropsSpeed = 1.0f; ///< Speed of rain drops as they fall through the air (Range: 0 to 100; Step: 0.1)
        float m_rainDropsLighting = 1.0f; ///< Brightness of the raindrops (Range: 0 to 100; Step: 0.1)
        float m_puddlesAmount = 1.5f; ///< Depth and Brightness of the puddles generated (Range 0 to 10; Step: 0.01)
        float m_puddlesMaskAmount = 1.0f; ///< Strength of the puddle mask to balance puddle results (Range 0 to 1; Step: 0.01)
        float m_puddlesRippleAmount = 2.0f; ///< Strength and frequency of the puddles (offset of Puddle amount) (Range: 0 to 100; Step: 0.1)
        float m_splashesAmount = 1.3f; ///< Strength of the splash effects generated by the rain (Range: 0 to 1000; Step: 0.1)

        static void Reflect(AZ::ReflectContext* context);

    private:
        static bool VersionConverter(AZ::SerializeContext& context,
            AZ::SerializeContext::DataElementNode& classElement);
    };

    /**
     * Updates the engine's rain settings based on the given
     * entity ID and rain options
     *
     * This lives outside the rain component so that the EditorRainComponent
     * can easily access the same functionality.
     *
     * @param worldTransform The transform where the rain effect is centered
     * @param options Rain options
     */
    void UpdateRainSettings(const AZ::Transform& worldTransform, RainOptions options);
    
    /**
     * This really just tells the 3D engine to use rain with an amount of 0
     * which is the way to turn off rain. Other rain components may 
     * apply rain settings after this which is expected. 
     */
    void TurnOffRain();

    class RainComponent
        : public AZ::Component
        , public AZ::TransformNotificationBus::Handler
        , public Rain::RainComponentRequestBus::Handler
    {
    public:
        friend class EditorRainComponent;

        AZ_COMPONENT(RainComponent, "{9EECE8BD-2355-4600-BEF0-49415CAF72E8}");

        ~RainComponent() override;

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // TransformBus
        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        // RainComponentRequestBus::Handler
        void Enable() override;
        void Disable() override;
        void Toggle() override;

        bool IsEnabled() override
        {
            return m_enabled;
        }

        /*
            Note: If a user needs to call many setters it's best to not use the 
            individual setters. Instead call GetRainOptions, modify the object
            and then call SetRainOptions. That way you're not calling UpdateRain 
            more than once. 
        */

        bool GetIgnoreVisAreas() override { return m_rainOptions.m_ignoreVisAreas; }
        void SetIgnoreVisAreas(bool ignoreVisAreas) override;

        bool GetDisableOcclusion() override { return m_rainOptions.m_disableOcclusion; }
        void SetDisableOcclusion(bool disableOcclusion) override;

        float GetRadius() override { return m_rainOptions.m_radius; }
        void SetRadius(float radius) override;

        float GetAmount() override { return m_rainOptions.m_amount; }
        void SetAmount(float amount) override;

        float GetDiffuseDarkening() override { return m_rainOptions.m_diffuseDarkening; }
        void SetDiffuseDarkening(float diffuseDarkening) override;

        float GetRainDropsAmount() override { return m_rainOptions.m_rainDropsAmount; }
        void SetRainDropsAmount(float rainDropsAmount) override;

        float GetRainDropsSpeed() override { return m_rainOptions.m_rainDropsSpeed; }
        void SetRainDropsSpeed(float rainDropsSpeed) override;

        float GetRainDropsLighting() override { return m_rainOptions.m_rainDropsLighting; }
        void SetRainDropsLighting(float rainDropsLighting) override;

        float GetPuddlesAmount() override { return m_rainOptions.m_puddlesAmount; }
        void SetPuddlesAmount(float puddlesAmount) override;

        float GetPuddlesMaskAmount() override { return m_rainOptions.m_puddlesMaskAmount; }
        void SetPuddlesMaskAmount(float puddlesMaskAmount) override;

        float GetPuddlesRippleAmount() override { return m_rainOptions.m_puddlesRippleAmount; }
        void SetPuddlesRippleAmount(float puddlesRippleAmount) override;

        float GetSplashesAmount() override { return m_rainOptions.m_splashesAmount; }
        void SetSplashesAmount(float splashesAmount) override;
        
        RainOptions GetRainOptions() override { return m_rainOptions; }
        void SetRainOptions(RainOptions rainOptions) override;

        /**
         * Sends this component's rain parameters to the engine
         *
         * If the component is disabled or the "amount" of this rain object is 0
         * the engine will attempt to disable rain. Rain can be re-enabled if another
         * rain component updates and overrides that setting.
         */
        void UpdateRain() override;

    protected:

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC("RainService"));
        }

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC("TransformService"));
        }

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC("RainService"));
        }

        static void Reflect(AZ::ReflectContext* context);

#ifndef NULL_RENDERER
        /**
         * Cache any rain textures
         *
         * Load any textures needed by the rain system and store the returned pointers. 
         * All textures will be released when the component gets destroyed (not deactivated).
         *
         * This is just to make sure that the rain textures are loaded before they're used.
         */
        void PrecacheTextures();
#endif

        // Reflected Data
        bool m_enabled = true;
        RainOptions m_rainOptions;

        //Unreflected Data
        AZ::Transform m_currentWorldTransform;

#ifndef NULL_RENDERER
        AZStd::vector<ITexture*> m_Textures;
#endif
    };
}