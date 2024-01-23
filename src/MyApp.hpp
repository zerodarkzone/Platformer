#ifndef PHYSICS_TEST_MYAPP_HPP
#define PHYSICS_TEST_MYAPP_HPP

#include <crogine/core/App.hpp>
#include <crogine/core/StateStack.hpp>

#define USE_SHAPE_USER_INFO

class MyApp final : public cro::App
{
public:
    MyApp();

    ~MyApp() override = default;

private:
    cro::StateStack m_stateStack;

    void handleEvent(const cro::Event&) override;

    void handleMessage(const cro::Message&) override;

    void simulate(float) override;

    void render() override;

    bool initialise() override;

    void finalise() override;
};

#endif // PHYSICS_TEST_MYAPP_HPP
