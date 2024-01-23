#ifndef PHYSICS_TEST_MENUSTATE_HPP
#define PHYSICS_TEST_MENUSTATE_HPP

#include "StateIDs.hpp"

#include <crogine/core/State.hpp>
#include <crogine/ecs/Scene.hpp>
#include <crogine/graphics/MeshResource.hpp>
#include <crogine/graphics/ShaderResource.hpp>
#include <crogine/graphics/MaterialResource.hpp>
#include <crogine/graphics/TextureResource.hpp>


/*!
Creates a state to render a menu.
*/
class MenuState final : public cro::State
{
public:
    MenuState(cro::StateStack&, cro::State::Context);

    ~MenuState() override = default;

    cro::StateID getStateID() const override
    {
        return States::MainMenu;
    }

    bool handleEvent(const cro::Event&) override;

    void handleMessage(const cro::Message&) override;

    bool simulate(float) override;

    void render() override;

private:
    cro::Scene m_scene;
    cro::MeshResource m_meshResource;
    cro::ShaderResource m_shaderResource;
    cro::MaterialResource m_materialResource;
    cro::TextureResource m_textureResource;

    void addSystems();

    void loadAssets();

    void createScene();
};

#endif // PHYSICS_TEST_MENUSTATE_HPP
