/* $Id$
 *
 * ruffina, 2004
 */
#pragma interface

#include "schedulertask.h"

class Object;

/**
 * @short ������ ��� ������������, ������� �� ���� �������� �������
 * @author Igor S. Petrenko
 * @see Scheduler
 * @see SchedulerTask
 */
struct SchedulerTaskRoundObject : public virtual SchedulerTask
{
	typedef ::Pointer<SchedulerTaskRoundObject> Pointer;
	
	virtual void run( );
	/** ���������� ������ */
	virtual void run( Object* object ) = 0;
	virtual int getPriority( ) const;
};
