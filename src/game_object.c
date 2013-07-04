#include <LCUI_Build.h>
#include LC_LCUI_H
#include LC_WIDGET_H
#include "game_object.h"

#include <time.h>

#define PAUSE	0
#define PLAY	1

/** ������¼ */
typedef struct ActionRec_ {
	int obj_id;		/**< ����ı�ʶ�� */
	ActionData *action;	/**< ��Ӧ�Ķ����� */
} ActionRec;

/** ������״̬ */
typedef struct ActionStatus_ {
	int obj_id;		/**< �����ʶ�� */
	int state;		/**< ״̬��ָʾ���Ż�����ͣ */
	int current;		/**< ��¼��ǰ֡��������ţ�֡��Ŵ�1��ʼ */
	ActionData *action;	/**< ��Ӧ�Ķ����� */
	LCUI_Func func;		/**< �������Ļص����� */
} ActionStatus;

typedef struct GameObject_ {
	int state;			/**< ��ǰ״̬ */
	int x, y;			/**< �����е����� */
	int w, h;			/**< ���������ĳߴ� */
	int global_bottom_line_y;	/**< ���ߵ�Y������ */
	int global_center_x;		/**< ���ĵ��X������ */
	LCUI_BOOL data_valid;		/**< ��ǰʹ�õ������Ƿ���Ч */
	ActionRec *current;		/**< ��ǰ�������� */
	LCUI_Queue action_list;		/**< �����б� */
} GameObject;

static LCUI_Queue action_database;
static LCUI_Queue action_stream;

static int database_init = FALSE;
static int frame_proc_timer = -1;

static void ActionStatus_Destroy( void *arg )
{

}

static void ActionStatus_Init( ActionStatus *p_status )
{
	p_status->obj_id = 0;
	p_status->action = NULL;
	p_status->current = 0;
	p_status->state = PAUSE;
	p_status->func.func = NULL;
}

static void Action_CallFunc( ActionStatus *p_status )
{
	AppTasks_CustomAdd( ADD_MODE_REPLACE | AND_ARG_S, &p_status->func );
}

static void ActionData_Destroy( void *arg )
{
	ActionData *action;
	action = (ActionData*)arg;
	Queue_Destroy( &action->frame );
}

static ActionStatus* Action_GetStatus( ActionData *action, int obj_id )
{
	int i, n;
	ActionStatus *p_status;

	if( action == NULL ) {
		return NULL;
	}
	Queue_Lock( &action_stream );
	n = Queue_GetTotal( &action_stream );
	for( i=0; i<n; ++i ) {
		p_status = (ActionStatus*)Queue_Get( &action_stream, i );
		if( !p_status || p_status->action != action ) {
			continue;
		}
		if( obj_id == p_status->obj_id ) {
			Queue_Unlock( &action_stream );
			return p_status;
		}
	}
	Queue_Unlock( &action_stream );
	return NULL;
}

/**
 * ����һ��������
 * �����Ķ���������¼����������
 * @returns
 *	�����򷵻�ָ�������еĸö�������ָ�룬ʧ���򷵻�NULL
 */
LCUI_API ActionData* Action_Create( void )
{
	int pos;
	ActionData *p, action;

	Queue_Init( &action.frame, sizeof(ActionFrameData), NULL );
	
	if( !database_init ) {
		Queue_Init(	&action_database,
				sizeof(ActionData),
				ActionData_Destroy );
		database_init = TRUE;
	}
	Queue_Lock( &action_database );
	/* ��¼�ö��������� */
	pos = Queue_Add( &action_database, &action );
	p = (ActionData*)Queue_Get( &action_database, pos );
	Queue_Unlock( &action_database );
	DEBUG_MSG("create action: %p\n", p);
	return p;
}

static int ActionStream_Delete( ActionData *action )
{
	int n;
	ActionData* tmp;

	Queue_Lock( &action_stream );
	n = Queue_GetTotal( &action_stream );
	for(n; n>=0; --n) {
		tmp = (ActionData*)Queue_Get( &action_stream, n );
		if( tmp == action ) {
			Queue_Delete( &action_stream, n );
			break;
		}
	}
	Queue_Unlock( &action_stream );
	if( n < 0 ) {
		return -1;
	}
	return 0;
}

/**
 * ɾ��һ������
 * �Ӷ�������ɾ��ָ���Ķ���
 * @param action
 *	��ɾ���Ķ���
 * @returns
 *	�����򷵻�0��ʧ���򷵻�-1
 */
LCUI_API int Action_Delete( ActionData* action )
{
	int n;
	ActionData* tmp;

	ActionStream_Delete( action );

	Queue_Lock( &action_database );
	n = Queue_GetTotal( &action_database );
	for(n; n>=0; --n) {
		tmp = (ActionData*)Queue_Get( &action_database, n );
		if( tmp == action ) {
			Queue_Delete( &action_database, n );
			break;
		}
	}
	Queue_Unlock( &action_database );
	if( n < 0 ) {
		return -1;
	}
	return 0;
}

/**
 * Ϊ���������ص�����
 * �����ص������󣬶���ÿ����һ֡������øú���
 * @param action
 *	Ŀ�궯��
 * @param obj_id
 *	ʹ�øö����Ķ����ID
 * @param func
 *	ָ��ص������ĺ���ָ��
 * @param arg
 *	�贫�ݸ��ص������ĵڶ�������
 * @returns
 *	�����򷵻�0��ʧ���򷵻�-1
 * @warning
 *	�����ڶ�������һ�β��ź���ô˺������������Ҳ����ö����Ĳ���ʵ�������¹���ʧ��
 * */
LCUI_API int Action_Connect(	ActionData *action,
				int obj_id,
				void (*func)(void*),
				void *arg )
{
	ActionStatus *p_status;

	if( !action ) {
		return -1;
	}
	p_status = Action_GetStatus( action, obj_id );
	if( p_status == NULL ) {
		return -1;
	}
	p_status->func.func = (CallBackFunc)func;
	p_status->func.id = LCUIApp_GetSelfID();
	p_status->func.arg[0] = arg;
	p_status->func.arg[1] = NULL;
	p_status->func.destroy_arg[0] = FALSE;
	p_status->func.destroy_arg[1] = FALSE;
	return 0;
}

static void ActionStream_Sort(void)
{
	int i, j, pos, total;
	ActionStatus *p_status;
	ActionFrameData *p, *q;

	Queue_Lock( &action_stream );
	total = Queue_GetTotal( &action_stream );
	for(i=0; i<total; ++i) {
		p_status = (ActionStatus*)Queue_Get( &action_stream, i );
		if( !p_status ) {
			continue;
		}
		if(p_status->current > 0) {
			pos = p_status->current-1;
		} else {
			pos = 0;
		}
		p = (ActionFrameData*)Queue_Get( &p_status->action->frame, pos );
		if( !p ) {
			continue;
		}

		for(j=i+1; j<total; ++j) {
			p_status = (ActionStatus*)Queue_Get( &action_stream, j );
			if( !p_status ) {
				continue;
			}
			if(p_status->current > 0) {
				pos = p_status->current-1;
			} else {
				pos = 0;
			}
			q = (ActionFrameData*)Queue_Get( &p_status->action->frame, pos );
			if( !q ) {
				continue;
			}
			if( q->current_time < p->current_time ) {
				Queue_Swap( &action_stream, j, i );
			}
		}
	}
	Queue_Unlock( &action_stream );
}

static void ActionStream_TimeSub( int time )
{
	ActionFrameData *frame;
	ActionStatus *p_status;
	int i, total, pos;

	Queue_Lock( &action_stream );
	total = Queue_GetTotal(&action_stream);
	DEBUG_MSG("start\n");
	for(i=0; i<total; ++i) {
		p_status = (ActionStatus*)Queue_Get( &action_stream, i );
		if( !p_status || p_status->state == PAUSE ) {
			continue;
		}
		if( p_status->current > 0 ) {
			pos = p_status->current-1;
		} else {
			pos = 0;
		}
		frame = (ActionFrameData*)
			Queue_Get( &p_status->action->frame, pos );
		if( !frame ) {
			continue;
		}
		frame->current_time -= time;
		DEBUG_MSG("action: %p, current: %d, time:%ld, sub:%d\n",
			p_status->action, pos, frame->current_time, time);
	}
	DEBUG_MSG("end\n");
	Queue_Unlock( &action_stream );
}

/** �������еĶ�������һ֡������ȡ��ǰ�����͵�ǰ֡�ĵȴ�ʱ�� */
static ActionStatus* ActionStream_Update( int *sleep_time )
{
	int i, total;
	ActionStatus *action_status, *temp = NULL;
	ActionFrameData *frame = NULL;

	DEBUG_MSG("start\n");
	total = Queue_GetTotal(&action_stream);
	for(i=0; i<total; ++i){
		action_status = (ActionStatus*)
				Queue_Get( &action_stream, i );
		if(action_status->state == PLAY) {
			break;
		}
	}
	if(i >= total || !action_status ) {
		return NULL;
	}
	/*
	 * ������Щ������δ���µ�һ֡ͼ�񣬶��������ͼ��Ҳδ�����һ֡��ͼ����ˣ�
	 * ��Ҫ���ȴ���֡���Ϊ0�Ķ�����
	 * */
	for(i=0; i<total; ++i){
		temp = (ActionStatus*)Queue_Get( &action_stream, i );
		if( action_status->state == PLAY && temp->current == 0 ) {
			action_status = temp;
			break;
		}
	}
	if( action_status && action_status->current > 0 ) {
		frame = (ActionFrameData*)
			Queue_Get(	&action_status->action->frame,
					action_status->current-1 );
		if( !frame ) {
			return NULL;
		}
		DEBUG_MSG("current time: %ld\n", frame->current_time);
		if(frame->current_time > 0) {
			*sleep_time = frame->current_time;
			ActionStream_TimeSub( frame->current_time );
		}

		frame->current_time = frame->sleep_time;
		++action_status->current;
		total = Queue_GetTotal( &action_status->action->frame );
		if( action_status->current > total ) {
			action_status->current = 1;
		}
		frame = (ActionFrameData*)
			Queue_Get(	&action_status->action->frame,
					action_status->current-1 );
		if( !frame ) {
			return NULL;
		}
	} else {
		action_status->current = 1;
		frame = (ActionFrameData*)
			Queue_Get( &action_status->action->frame, 0 );
	}
	ActionStream_Sort();
	DEBUG_MSG("current frame: %d\n", action_status->current);
	DEBUG_MSG("end\n");
	return action_status;
}

static void Process_Frames( void )
{
	int sleep_time = 10;
	ActionStatus *action_status;

	while(!LCUI_Active()) {
		LCUI_MSleep(10);
	}
	action_status = ActionStream_Update( &sleep_time );
	LCUITimer_Reset( frame_proc_timer, sleep_time );
	if( action_status ) {
		Action_CallFunc( action_status );
	}
}

/**
/* ����ָ��ʵ�������еĶ���
 * @param action
 *	Ҫ���ŵĶ���
 * @param obj_id
 *	����ʵ����Ӧ�ı�ʶ�ţ���������0�����Ϊ�ö�������һ���µĶ���ʵ��
 * @returns
 *	�����򷵻ظö����Ĳ��ű�ʶ�ţ�ʧ���򷵻�-1
 */
LCUI_API int Action_Play( ActionData *action, int obj_id )
{
	static int new_obj_id = 1;
	ActionStatus new_status, *p_status;

	if( !action ) {
		return -1;
	}
	if( frame_proc_timer == -1 ) {
		Queue_Init(
			&action_stream,
			sizeof(ActionStatus),
			ActionStatus_Destroy
		);
		frame_proc_timer = LCUITimer_Set( 50, Process_Frames, TRUE );
	}
	if( obj_id <= 0 ) {
		ActionStatus_Init( &new_status );
		new_status.obj_id = new_obj_id++;
		new_status.action = action;
		new_status.state = PLAY;
		Queue_Add( &action_stream, &new_status );
		return new_status.obj_id;
	}
	p_status = Action_GetStatus( action, obj_id );
	if( p_status == NULL ) {
		return -1;
	}
	p_status->state = PLAY;
	return obj_id;
}

/**
 * ��ָͣ��ʵ�������еĶ���
 * @param action
 *	Ҫ��ͣ�Ķ���
 * @param obj_id
 *	��ö����Ĳ���ʵ����Ӧ�ı�ʶ��
 * @returns
 *	�����򷵻ظö����Ĳ��ű�ʶ�ţ�ʧ���򷵻�-1
 */
LCUI_API int Action_Pause( ActionData *action, int obj_id )
{
	static int new_play_id = 1;
	ActionStatus *p_status;

	if( !action ) {
		return -1;
	}
	if( frame_proc_timer == -1 ) {
		return -1;
	}
	if( obj_id <= 0 ) {
		return -1;
	}
	p_status = Action_GetStatus( action, obj_id );
	if( p_status == NULL ) {
		return -1;
	}
	p_status->state = PAUSE;
	return 0;
}

static void GameObject_RefreshFrame( void *arg )
{
	LCUI_Widget *widget = (LCUI_Widget*)arg;
	DEBUG_MSG("refresh\n");
	Widget_Draw( widget );
}

/**
 * �л�����Ķ���
 * @param widget
 *	Ŀ��ActiveBox����
 * @param action
 *	�л������¶���
 * @return
 *	�л��ɹ��򷵻�0��δ�ҵ�ָ��ID�Ķ�����¼���򷵻�-1
 */
LCUI_API int GameObject_SwitchAction(	LCUI_Widget *widget,
					ActionData *action )
{
	int i, n;
	ActionRec *p_rec;
	GameObject *obj;

	obj = (GameObject*)Widget_GetPrivData( widget );
	n = Queue_GetTotal( &obj->action_list );
	for(i=0; i<n; ++i) {
		p_rec = (ActionRec*)
			Queue_Get( &obj->action_list, i );

		if( !p_rec || p_rec->action != action ) {
			continue;
		}
		if( obj->current ) {
			Action_Pause(
				obj->current->action,
				obj->current->obj_id
			);
		}
		if( obj->state == PLAY ) {
			Action_Play(
				p_rec->action,
				p_rec->obj_id
			);
		} else {
			Action_Pause(
				p_rec->action,
				p_rec->obj_id
			);
		}
		obj->current = p_rec;
		return 0;
	}
	return -1;
}

/** ���Ŷ���Ķ��� */
LCUI_API int GameObject_PlayAction( LCUI_Widget *widget )
{
	int ret;
	GameObject *obj;

	obj = (GameObject*)Widget_GetPrivData(widget);
	obj->state = PLAY;
	if( obj->current == NULL ) {
		obj->current = (ActionRec*)Queue_Get(
					&obj->action_list, 0 );
		if( obj->current == NULL ) {
			return -1;
		}
	}
	ret = Action_Play(	obj->current->action,
				obj->current->obj_id );
	if( ret > 0 ) {
		/* ���沥�ű�ʶ�� */
		obj->current->obj_id = ret;
		/* �����ص����� */
		Action_Connect(
			obj->current->action,
			obj->current->obj_id,
			GameObject_RefreshFrame,
			(void*)widget
		);
		return 0;
	}
	return -1;
}

/** ��ͣ����Ķ��� */
LCUI_API int GameObject_PauseAction( LCUI_Widget *widget )
{
	GameObject *obj;

	obj = (GameObject*)Widget_GetPrivData(widget);
	obj->state = PAUSE;
	if( obj->current == NULL ) {
		obj->current = (ActionRec*)Queue_Get(
					&obj->action_list, 0 );
		if( obj->current == NULL ) {
			return -1;
		}
	}
	return Action_Pause(	obj->current->action,
				obj->current->obj_id );
}

LCUI_API int Action_AddFrame(	ActionData* action,
				int offset_x,
				int offset_y,
				LCUI_Graph *graph,
				long int sleep_time )
{
	ActionFrameData frame;
	
	frame.current_time = sleep_time;
	frame.sleep_time = sleep_time;
	frame.offset.x = offset_x;
	frame.offset.y = offset_y;
	frame.graph = *graph;

	if( 0 <= Queue_Add( &action->frame, &frame ) ) {
		return 0;
	}
	return -1;
}

static void GameObject_ExecInit( LCUI_Widget *widget )
{
	GameObject *obj;

	obj = (GameObject*)Widget_NewPrivData( widget, sizeof(GameObject) );
	Queue_Init( &obj->action_list, sizeof(ActionRec), NULL );
	obj->current = NULL;
	obj->global_bottom_line_y = 0;
	obj->global_center_x = 0;
	obj->data_valid = FALSE;
	obj->state = 0;
	obj->x = 0;
	obj->y = 0;
	obj->w = 0;
	obj->h = 0;
}

/** ��ȡ��ǰ������Ϣ */
static int GameObject_UpdateData( LCUI_Widget *widget )
{
	int i, n;
	GameObject *obj;
	LCUI_Queue *frame_list;
	ActionFrameData *frame;
	int frame_bottom_y, frame_center_x;
	int box_w=0, box_h=0, point_x=0, point_y=0;

	obj = (GameObject*)Widget_GetPrivData( widget );
	if( obj->current == NULL ) {
		return -1;
	}
	frame_list = &obj->current->action->frame;
	n = Queue_GetTotal( &obj->current->action->frame );
	for( i=0; i<n; ++i ) {
		frame = (ActionFrameData*)Queue_Get( frame_list, i );
		if( frame == NULL ) {
			continue;
		}
		/*
		 * frame->graph�ǵ�ǰ֡����������
		 * frame->graph.w �� frame->graph.h ���Ǹ������ĳߴ�
		 * box_w �� box_h �Ǹö������������ߴ�
		 * frame_bottom_y �Ƕ�������ڵ�ǰ֡�������е�Y������
		 * frame_center_x �Ƕ����е��ڵ�ǰ֡�������е�X������
		 * point_y �� point_x �������������ƣ�����������Զ�����������
		 */
		frame_bottom_y = frame->graph.h + frame->offset.y;
		frame_center_x = frame->graph.w/2 + frame->offset.x;
		/* �Ա��������˵����ߵľ��� */
		if( frame_bottom_y > point_y ) {
			box_h = (box_h - point_y) + frame_bottom_y;
			point_y = frame_bottom_y;
		}
		/* �Աȵ��ߵ������׶˵ľ��� */
		if( frame->graph.h - frame_bottom_y > box_h - point_y  ) {
			box_h = point_y + (frame->graph.h - frame_bottom_y);
		}
		/* �Ա�������˵����ĵ�ľ��� */
		if( frame_center_x > point_x ) {
			box_w = (box_w - point_x) + frame_center_x;
			point_x = frame_center_x;
		}
		/* �Ա����ĵ㵽�����Ҷ˵ľ��� */
		if( frame->graph.w - frame_center_x > box_w - point_x ) {
			box_w = point_x + (frame->graph.w - frame_center_x);
		}
	}
	obj->global_center_x = point_x;
	obj->global_bottom_line_y = point_y;
	obj->w = box_w;
	obj->h = box_h;
	return 0;
}

/** Ϊ�������һ������ */
LCUI_API int GameObject_AddAction( LCUI_Widget *widget, ActionData *action )
{
	int ret;
	ActionRec rec;
	GameObject *obj;

	rec.action = action;
	rec.obj_id = 0;
	obj = (GameObject*)Widget_GetPrivData( widget );
	/* ����������б��� */
	ret = Queue_Add( &obj->action_list, &rec );
	if( ret < 0 ) {
		return -1;
	}
	return 0;
}

static void GameObject_ExecUpdate( LCUI_Widget *widget )
{
	GameObject *obj;
	LCUI_Pos pos;
	obj = (GameObject*)Widget_GetPrivData( widget );
	if( obj->current == NULL ) {
		obj->current = (ActionRec*)Queue_Get( &obj->action_list, 0 );
		/* ��Ȼû�ж����������ǾͲ����и��� */
		if( obj->current == NULL ) {
			return;
		}
	}
	/* ������ݻ���Ч */
	if( obj->data_valid ) {
		return;
	}
	/* ������ݸ��³ɹ� */
	if( GameObject_UpdateData( widget ) == 0 ) {
		/* ���㲿�������� */
		pos.x = obj->x - obj->global_center_x;
		pos.y = obj->y - obj->global_bottom_line_y;
		/* �ƶ�������λ�� */
		Widget_Move( widget, pos );
		/* �����������ĳߴ磬��������ʾ����Ķ��� */
		Widget_Resize( widget, Size(obj->w, obj->h) );
		obj->data_valid = TRUE;
	}
}

static void GameObject_ExecDraw( LCUI_Widget *widget )
{
	GameObject *obj;
	LCUI_Pos pos;
	ActionStatus *p_status;
	ActionFrameData *frame;
	LCUI_Graph *graph;

	obj = (GameObject*)Widget_GetPrivData( widget );
	if( obj->current == NULL ) {
		obj->current = (ActionRec*)Queue_Get( &obj->action_list, 0 );
		/* ��Ȼû�ж����������ǾͲ����и��� */
		if( obj->current == NULL ) {
			return;
		}
	}
	p_status = Action_GetStatus(
			obj->current->action,
			obj->current->obj_id
	);
	/* ��ȡ��ǰ֡����ͼ�� */
	frame = (ActionFrameData*)Queue_Get(
			&p_status->action->frame,
			p_status->current-1 
	);
	if( frame == NULL ) {
		return;
	}
	/* ���㵱ǰ֡����ڲ��������� */
	pos.y = frame->graph.h + frame->offset.y;
	pos.y = obj->global_bottom_line_y - pos.y;
	pos.x = frame->graph.w/2 + frame->offset.x;
	pos.x = obj->global_center_x - pos.x;
	/* ��ȡ��������ͼ���ͼ�� */
	graph = Widget_GetSelfGraph( widget );
	/* Ȼ������ȥ */
	Graph_Replace( graph, &frame->graph, pos );
}

/** �ƶ���Ϸ�����λ�� */
LCUI_API void GameObject_Move( LCUI_Widget *widget, int x, int y )
{
	GameObject *obj;

	obj = (GameObject*)Widget_GetPrivData( widget );
	obj->x = x;
	obj->y = y;
	x = obj->x - obj->global_center_x;
	y = obj->y - obj->global_bottom_line_y;
	Widget_Move( widget, Pos(x,y) );
}

LCUI_API LCUI_Widget* GameObject_New(void)
{
	return Widget_New("GameObject");
}


LCUI_API void GameObject_Register(void)
{
	WidgetType_Add("GameObject");
	WidgetFunc_Add("GameObject", GameObject_ExecInit, FUNC_TYPE_INIT );
	WidgetFunc_Add("GameObject", GameObject_ExecDraw, FUNC_TYPE_DRAW );
	WidgetFunc_Add("GameObject", GameObject_ExecUpdate, FUNC_TYPE_UPDATE );
}
