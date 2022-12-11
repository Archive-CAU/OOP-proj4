////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: ��â�� Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <array>

#include "CSphere.h"
#include "CWall.h"
#include "CLight.h"
#include "CTopWall.h"
#include "CBottomWall.h"
#include "CRightWall.h"
#include "CLeftWall.h"
#include "CFloor.h"
#include "CHole.h"
#include "CHandSphere.h"
#include "Status.h"
#include "Player.h"
#include "d3dUtility.h"
#include "d3dfont.h"

#include "Platform.h"
#include "Jumper.h"
#include "DisplayText.h"
#include "Map.h"



using std::array;

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------

// window size
const int Width = 1920;
const int Height = 1080;

HWND window;

IDirect3DDevice9* Device = NULL;

#define NUM_PLATFORM 16

bool isGameOver = false;

Jumper g_jumper;
CLight g_light;
CSphere g_sphere;

D3DLIGHT9 lit;

Player players[2] = { Player(1), Player(2) };
vector<Player*> playerVec = { &players[0], &players[1] };
Status status(playerVec);

DisplayText displayText(Width, Height);

Map map1(16), map2(16), map3(16);

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup() {
    int i;

    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);

    if (!displayText.create("Times New Roman", 16, Device)) return false;

    map1.setPosition(0, 0, 0, 0);
    map1.setPosition(1, -0.5, 0, 0.3);
    map1.setPosition(2, +0.5, 0, 0.3);
    map1.setPosition(3, -1.0, 0, 0.6);
    map1.setPosition(4, +1.0, 0, 0.6);
    map1.setPosition(5, -1.5, 0, 0.9);
    map1.setPosition(6, +1.5, 0, 0.9);
    map1.setPosition(7, -2.0, 0, 1.2);
    map1.setPosition(8, +2.0, 0, 1.2);
    map1.setPosition(9, +1.5, 0, 1.5);
    map1.setPosition(10, -1.5, 0, 1.5);
    map1.setPosition(11, +1.0, 0, 1.8);
    map1.setPosition(12, -1.0, 0, 1.8);
    map1.setPosition(13, +0.5, 0, 2.1);
    map1.setPosition(14, -0.5, 0, 2.1);
    map1.setPosition(15, +0.0, 0, 2.4);

    // create platform

    /*for (int i = 0; i < NUM_PLATFORM; i++) {
       if (!map1.g_platforms[i].create(Device, d3d::GREEN)) return false;
       D3DXVECTOR3 m = map1.g_platforms[i].getPosition();
       map1.g_platforms[i].setPosition(m.x, m.y, m.z);
    }*/
    for (int i = 0; i < NUM_PLATFORM; i++) {
        map1.g_platforms[i].create(Device, d3d::GREEN);
    }

    // create jumper
    if (!g_jumper.create(Device)) return false;
    g_jumper.setPosition(0, 0.005, 1);
    g_jumper.setVelocity(0, 0);

    // light setting 
    D3DXVECTOR3 m = g_jumper.getPosition();
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type = D3DLIGHT_POINT;
    lit.Diffuse = d3d::WHITE;
    lit.Specular = d3d::WHITE * 1.0f;
    lit.Ambient = d3d::WHITE * 1.0f;
    lit.Position = D3DXVECTOR3(m.x, 3.0f, m.z);
    lit.Range = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;

    float radius = 0.01f;
    if (false == g_light.create(Device, lit, radius)) return false;

    // Position and aim the camera.
    D3DXVECTOR3 pos(m.x, 3.0f, m.z);
    D3DXVECTOR3 target(m.x, 0.0f, m.z);
    D3DXVECTOR3 up(0.0f, 0.0f, 1.0f);   // camera's rotation
    D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
    Device->SetTransform(D3DTS_VIEW, &g_mView);

    // Set the projection matrix.
    D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4, (float)Width / (float)Height, 1.0f, 100.0f);
    Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

    g_light.setLight(Device, g_mWorld);
    return true;
}

// set of destroy function
void Cleanup(void)
{
    for (int i = 0; i < NUM_PLATFORM; i++) {
        //g_platforms[i].destroy();
        map1.g_platforms[i].destroy();
    }
    g_light.destroy();
    displayText.destory();
}

// Update
bool Display(float timeDelta)
{
    int i = 0;
    int j = 0;

    if (Device)
    {
        Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
        Device->BeginScene();

        displayText.update();

        // draw platforms
        for (int i = 0; i < NUM_PLATFORM; i++) {
            //g_platforms[i].draw(Device, g_mWorld);
            map1.g_platforms[i].draw(Device, g_mWorld);
        }

        // intersect between jumper and platform
        if (isGameOver) {
            g_jumper.setPosition(0, 0.01, 3);
            g_jumper.setVelocity(0, 0);
        }
        else {
            g_jumper.jumperUpdate(timeDelta);
        }
        for (int i = 0; i < NUM_PLATFORM; i++) {
            if (g_jumper.hasIntersected(map1.g_platforms[i])) {
                g_jumper.setVelocity(g_jumper.getVelocity_X(), 0);
                if (g_jumper.isFirstTouch()) {
                    g_jumper.setFirstTouch(false);
                    g_jumper.whereIdx = i;
                }
                else {
                    g_jumper.setOnPlatform(true);
                }
            }
            else if (g_jumper.whereIdx == i) {
                g_jumper.setVelocity(g_jumper.getVelocity_X(), g_jumper.getVelocity_Z());
                g_jumper.setOnPlatform(false);
                g_jumper.setFirstTouch(true);
                g_jumper.whereIdx = -1;
            }
        }

        // draw jumper
        g_jumper.draw(Device, g_mWorld);

        D3DXVECTOR3 m = g_jumper.getPosition();
        D3DXVECTOR3 pos(m.x, 3.0f, m.z);
        D3DXVECTOR3 target(m.x, 0.0f, m.z);
        D3DXVECTOR3 up(0.0f, 0.0f, 1.0f);   // camera's rotation
        D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
        Device->SetTransform(D3DTS_VIEW, &g_mView);

        lit.Position = D3DXVECTOR3(m.x, 3.0f, m.z);
        Device->SetLight(0, &lit);

        Device->EndScene();
        Device->Present(0, 0, 0, 0);
        Device->SetTexture(0, NULL);
    }

    if (!isGameOver && g_jumper.getPosition().z < -3) {
        isGameOver = true;
    }

    return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static bool wire = false;
    static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

    switch (msg) {
    case WM_DESTROY: {
        ::PostQuitMessage(0);
        break;
    }
    case WM_KEYDOWN: {
        switch (wParam) {
        case VK_ESCAPE:
            ::DestroyWindow(hwnd);
            break;
        case VK_RETURN:
            if (NULL != Device) {
                wire = !wire;
                Device->SetRenderState(D3DRS_FILLMODE,
                    (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
            }
            break;
        case VK_SPACE:
            if (g_jumper.isOnPlatform()) {
                D3DXVECTOR3 m = g_jumper.getPosition();
                g_jumper.setPosition(m.x, m.y, m.z + 0.05);
                g_jumper.setVelocity(g_jumper.getVelocity_X(), 0.5);
                g_jumper.setOnPlatform(false);
                g_jumper.setFirstTouch(true);
                g_jumper.whereIdx = -1;
            }
            break;
        case VK_LEFT:
            if (true) {
                g_jumper.setVelocity(-0.2, g_jumper.getVelocity_Z());
                g_jumper.setMoveState(MOVESTATE::LEFT);
            }
            break;
        case VK_RIGHT:
            if (true) {
                g_jumper.setVelocity(0.2, g_jumper.getVelocity_Z());
                g_jumper.setMoveState(MOVESTATE::RIGHT);
            }
            break;
        case 0x59:  // Y
            if (isGameOver) {
                isGameOver = false;
            }
            break;
        case 0x4E:  // N
            if (isGameOver) {
                //gameover
                //todo
            }
            break;
        }
        break;
    }
    case WM_KEYUP: {
        switch (wParam) {
        case VK_LEFT:
            if (g_jumper.getMoveState() == MOVESTATE::LEFT) {
                g_jumper.setVelocity(0, g_jumper.getVelocity_Z());
                g_jumper.setMoveState(MOVESTATE::STOP);
            }
        case VK_RIGHT:
            if (g_jumper.getMoveState() == MOVESTATE::RIGHT) {
                g_jumper.setVelocity(0, g_jumper.getVelocity_Z());
                g_jumper.setMoveState(MOVESTATE::STOP);
            }
        }
        break;
    }
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
    HINSTANCE prevInstance,
    PSTR cmdLine,
    int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));

    if (!d3d::InitD3D(hinstance,
        Width, Height, true, D3DDEVTYPE_HAL, &Device))
    {
        ::MessageBox(0, "InitD3D() - FAILED", 0, 0);
        return 0;
    }

    if (!Setup())
    {
        ::MessageBox(0, "Setup() - FAILED", 0, 0);
        return 0;
    }

    d3d::EnterMsgLoop(Display);

    Cleanup();

    Device->Release();

    return 0;
}