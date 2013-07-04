#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

/*
  ��ǰ֡�����ĵ��Y��ƽ����
         |
  _______|______
  |   O        |         ���ο����ǰ֡�ķ�Χ
  |  /|\       |
  |__/_\_______|________ ���ߵ�X��ƽ����
      |
      |
  ������е��Y��ƽ����
*/

/** һ֡��������Ϣ */
typedef struct ActionFrameData_ {
	LCUI_Pos offset;	/**< ��ǰ��������е�����ڸ�֡ͼ������е��ƫ���� */
	LCUI_Graph graph;	/**< ��ǰ֡��ͼ�� */
	long int sleep_time;	/**< ��֡��ʾ��ʱ������λ�����룩 */
	long int current_time;	/**< ��ǰʣ�µĵȴ�ʱ�� */
} ActionFrameData;

/** ����������Ϣ */
typedef struct ActionData_ {
	LCUI_Queue frame;	/**< ���ڼ�¼�ö�����������֡���� */
} ActionData;

/**
 * ����һ��������
 * �����Ķ���������¼����������
 * @returns
 *	�����򷵻�ָ�������еĸö�������ָ�룬ʧ���򷵻�NULL
 */
LCUI_API ActionData* Action_Create( void );

/**
 * ɾ��һ������
 * �Ӷ�������ɾ��ָ���Ķ���
 * @param action
 *	��ɾ���Ķ���
 * @returns
 *	�����򷵻�0��ʧ���򷵻�-1
 */
LCUI_API int Action_Delete( ActionData* action );

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
				void *arg );

/**
/* ����ָ��ʵ�������еĶ���
 * @param action
 *	Ҫ���ŵĶ���
 * @param obj_id
 *	����ʵ����Ӧ�ı�ʶ�ţ���������0�����Ϊ�ö�������һ���µĶ���ʵ��
 * @returns
 *	�����򷵻ظö����Ĳ��ű�ʶ�ţ�ʧ���򷵻�-1
 */
LCUI_API int Action_Play( ActionData *action, int obj_id );

/**
 * ��ָͣ��ʵ�������еĶ���
 * @param action
 *	Ҫ��ͣ�Ķ���
 * @param obj_id
 *	��ö����Ĳ���ʵ����Ӧ�ı�ʶ��
 * @returns
 *	�����򷵻ظö����Ĳ��ű�ʶ�ţ�ʧ���򷵻�-1
 */
LCUI_API int Action_Pause( ActionData *action, int obj_id );

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
					ActionData *action );

/** ���Ŷ���Ķ��� */
LCUI_API int GameObject_PlayAction( LCUI_Widget *widget );

/** ��ͣ����Ķ��� */
LCUI_API int GameObject_PauseAction( LCUI_Widget *widget );

LCUI_API int Action_AddFrame(	ActionData* action,
				int offset_x,
				int offset_y,
				LCUI_Graph *graph,
				long int sleep_time );

/** Ϊ�������һ������ */
LCUI_API int GameObject_AddAction( LCUI_Widget *widget, ActionData *action );

/** �ƶ���Ϸ�����λ�� */
LCUI_API void GameObject_Move( LCUI_Widget *widget, int x, int y );

LCUI_API LCUI_Widget* GameObject_New(void);

LCUI_API void GameObject_Register(void);

#endif