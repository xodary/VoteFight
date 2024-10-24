#pragma once

#ifdef _UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#define CONNECT_SERVER
#define NPC_RANGE 5

#define BUBBLE_WIDTH 2
#define BUBBLE_HEIGHT 2


constexpr int SECTOR_RANGE_ROW = 8;
constexpr int SECTOR_RANGE_COL = 8;

constexpr int W_WIDTH = 400;
constexpr int W_HEIGHT = 400;

#define EPSILON				    0.01f

#define FRAME_BUFFER_WIDTH		1920
#define FRAME_BUFFER_HEIGHT		1080
#define USE_PIX

#define TERRAIN_WIDTH			400
#define TERRAIN_HEIGHT			400
#define DEPTH_BUFFER_WIDTH	    (FRAME_BUFFER_WIDTH * 4)
#define DEPTH_BUFFER_HEIGHT     (FRAME_BUFFER_HEIGHT * 4)
#define VCT_SCENE_VOLUME_SIZE	32
#define VCT_MIPS 3

#define MAX_NPC_COUNT			15
#define MAX_LIGHTS              3
#define MAX_BONES				70
#define MAX_BGM_SOUNDS			4
#define MAX_SFX_SOUNDS			14

#define TEXTURE_MASK_ALBEDO_MAP 0x0001
#define TEXTURE_MASK_NORMAL_MAP 0x0002
#define TEXTURE_MASK_SHADOW_MAP 0x0004

#define DT						CTimeManager::GetInstance()->GetDeltaTime()

#define KEY_NONE(key)			CInputManager::GetInstance()->GetKeyState(key) == KEY_STATE::NONE
#define KEY_TAP(key)			CInputManager::GetInstance()->GetKeyState(key) == KEY_STATE::TAP
#define KEY_HOLD(key)			CInputManager::GetInstance()->GetKeyState(key) == KEY_STATE::HOLD
#define KEY_AWAY(key)			CInputManager::GetInstance()->GetKeyState(key) == KEY_STATE::AWAY
#define CURSOR					CInputManager::GetInstance()->GetCursor()
#define OLD_CURSOR				CInputManager::GetInstance()->GetOldCursor()

enum class STATE_ENUM { CPlayerIdleState = 0, CPlayerWalkState, CPlayerRunState, CPlayerLeftTurn, CPlayerRightTurn, };

enum class MSG_TYPE
{
	NONE = 0x0000,
	TITLE = 0x0001,
	INGAME = 0x0002,
	ENDING = 0x0004,
	CREDIT = 0x0008,
	TRIGGER = 0x0010,
	PLAYER1_WEAPON_SWAP = 0x0020,
	PLAYER2_WEAPON_SWAP = 0x0040,
	PLAYER_ATTACK = 0x0080,
	GUARD_ATTACK = 0x0100,
	DISCONNECTION = 0x0200,
	GAME_OVER = 0x0400,
	GAME_CLEAR = 0x0800,
	PLAYER1_BGM_SWAP = 0x1000,
	PLAYER2_BGM_SWAP = 0x2000,

	COUNT
};

enum class SCENE_TYPE
{
	TITLE,
	SELECT,
	LOGIN,
	GAME,
	END,

	COUNT
};

enum class GROUP_TYPE
{
	STRUCTURE,		
	PLAYER,			
	NPC,			
	MONSTER,		
	BULLET,			
	BILBOARD,
	GROUND_ITEM,
	SKYBOX,
	TERRAIN,
	ITEM,
	BOX,
	ONCE_ITEM,

	UI,
	Test,
	COUNT
};

enum class COMPONENT_TYPE
{
	STATE_MACHINE,
	RIGIDBODY,
	ANIMATOR,
	TRANSFORM,
	COLLIDER,
	SPRITE_RENDERER,

	COUNT
};

enum class CAMERA_TYPE
{
	MAIN,
	UI,
	LIGHT,

	COUNT
};

enum class WEAPON_TYPE
{
	PUNCH,
	PISTOL,
	AXE,

	COUNT
};

enum class ANIMATION_TYPE
{
	NONE,
	LOOP,
	ONCE,
	ONCE_REVERSE,

	COUNT
};

enum class ANIMATION_CLIP_TYPE
{
	PLAYER_IDLE = 0,
	PLAYER_WALK_FORWARD_AND_BACK,
	PLAYER_WALK_LEFT,
	PLAYER_WALK_RIGHT,
	PLAYER_RUN_FORWARD,
	PLAYER_RUN_LEFT,
	PLAYER_RUN_RIGHT,
	PLAYER_PUNCH,
	PLAYER_PISTOL_IDLE,
	PLAYER_SHOOT,
	PLAYER_DIE,
	NPC_IDLE = 0,
	NPC_WALK_FORWARD,
	NPC_RUN_FORWARD,
	NPC_SHOOT,
	NPC_HIT,
	NPC_DIE,

	COUNT
};

enum class TRIGGER_TYPE
{
	OPEN_DOOR,
	OPEN_ELEC_PANEL,
	SIREN,
	OPEN_GATE,

	COUNT
};

enum class RENDER_TYPE
{
	STANDARD,
	DEPTH_WRITE,

	COUNT
};

enum class LIGHT_TYPE
{
	POINT = 1,
	SPOT,
	DIRECTIONAL,

	COUNT
};

enum class ROOT_PARAMETER_TYPE
{
	GameFramework,
	CAMERA,
	GAME_SCENE,
	OBJECT,
	SPRITE,
	BONE_OFFSET,
	BONE_TRANSFORM,
	ALBEDO_MAP,
	NORMAL_MAP,
	CUBE_MAP,
	SHADOW_MAP,
	CUBE_MAP2,
	G_COLOR,
	G_NORMAL,
	G_WORLDPOS,

	COUNT
};

enum class TEXTURE_TYPE
{
	ALBEDO_MAP,
	NORMAL_MAP,
	CUBE_MAP,
	SHADOW_MAP,
	CUBE_MAP2,
	G_COLOR,
	G_NORMAL,
	G_WORLDPOS,

	COUNT
};

enum SOUND_TYPE
{
	// BGM
	TITLE_BGM = 0,
	SELECT_BGM,
	INGAME_BGM_1,
	INGAME_BGM_2,
	ENDING_BGM,

	WALK,
	RUN,
	IDLE,

	// SFX
	BUTTON_OVER,
	GET_PISTOL,
	PISTOL_SHOT,
	TREE,
	SWING,
	PUNCH,
	TAKEOUT,
	EXCHANGE,
	CHEER,
};

enum class DAMAGE_TYPE
{
	NONE,
	BULLET_DAMAGE,
	DROWN_DAMAGE,

};