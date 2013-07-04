#include <LCUI_Build.h>
#include LC_LCUI_H
#include LC_WIDGET_H
#include LC_ACTIVEBOX_H

#include "game_object.h"

enum PlayerState {
	STATE_STANCE,
	STATE_MOVE
};

typedef struct GamePlayer_ {
	int state;		/**< ��ҵ�ǰ״̬ */
	int hp, mp;		/**< ����ֵ�ͷ���ֵ */
	LCUI_Widget *object;	/**< ��Ϸ���� */
} GamePlayer;

GamePlayer player;

/** �����ɫ���ƶ�����������Դ */
static ActionData* ActionRes_LoadMove(void)
{
	int i;
	ActionData *action;
	LCUI_Graph img_move[8];
	char path[512];

	action = Action_Create();
	for(i=0; i<8; ++i) {
		Graph_Init( &img_move[i] );
		sprintf( path, "drawable/move-%02d.png", i+1 );
		Graph_LoadImage( path, &img_move[i] );
		Action_AddFrame( action, 0,0, &img_move[i], 100 );
	}
	return action;
}

/** �����ɫ��վ������������Դ */
static ActionData* ActionRes_LoadStance(void)
{
	int i;
	ActionData *action;
	LCUI_Graph img_stance[4];
	char path[512];
	
	action = Action_Create();
	for(i=0; i<4; ++i) {
		Graph_Init( &img_stance[i] );
		sprintf( path, "drawable/stance-%02d.png", i+1 );
		Graph_LoadImage( path, &img_stance[i] );
		Action_AddFrame( action, 3,0, &img_stance[i], 100 );
	}
	return action;
}

/** �����ɫ�Ķ���������Դ */
ActionData* LoadGamePlayerRes( int action_type )
{
	switch( action_type ) {
	case STATE_STANCE: return ActionRes_LoadStance();
	case STATE_MOVE: return ActionRes_LoadMove();
	}
	return NULL;
}

int Game_Init(void)
{
	ActionData* action;
	/* ע��GameObject���� */
	GameObject_Register();

	player.state = STATE_STANCE;
	player.hp = 100;
	player.mp = 0;
	/* ����GameObject���� */
	player.object = GameObject_New();
	/* ������Ϸ��ɫ��Դ */
	action = LoadGamePlayerRes( player.state );
	/* ���������������Ϸ���� */
	GameObject_AddAction( player.object, action );
	/* �ƶ���Ϸ��ɫ��λ�� */
	GameObject_Move( player.object, 300, 300 );
	Widget_Show( player.object );
	return 0;
}

int Game_Start(void)
{
	GameObject_PlayAction( player.object );
	return 0;
}

int Game_Pause(void)
{
	return 0;
}
