/*-----------------------------------------------------------------------

Matt Marchant 2020
http://trederia.blogspot.com

crogine application - Zlib license.

This software is provided 'as-is', without any express or
implied warranty.In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.

-----------------------------------------------------------------------*/

#pragma once

#include <cstdint>

namespace MaterialID
{
    /*enum
    {

    };*/
}

namespace MeshID
{
    /*enum
    {

    };*/
}

namespace GameModelID {}

namespace FontID {}

namespace ShaderID {}

namespace CommandID
{
    enum
    {
        Player1 = 0x1,
        AI = 0x2,
        Map = 0x4,
        MapChild = 0x8,
    };
}

namespace TextureID
{
    enum
    {
        //game
        Background = 1,
        Background2,
        Background3,
    };
}

namespace AnimationID
{
    enum Animation : short
    {
        None = -1,
        Idle,
        Walk,
        Run,
        Attack,
        AttackCombo,
        Die,
        Dead,
        PrepareJump,
        Jump,
        ReloadJump,
        Fall,
        Land,
        StartSlide,
        Slide,
        EndSlide,
        WallSlide,
        Count
    };
}

namespace SpriteID
{
    enum : int8_t
    {
        Player = 0,
        Count
    };
}
