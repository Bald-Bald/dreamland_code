/* $Id$
 *
 * ruffina, 2004
 */
bool Rideable::canMount( Character *rider )
{
    Skill *skill = getRidingSkill( );

    if (!skill->usable( rider )) {
	rider->println("�� �� ������ �������� �� ����� ���������.");
	return false;
    }

    if (ch->leader && ch->leader != rider) {
	if (!gsn_steal_mount->usable( rider )) {
	    rider->pecho("%^C1 - ����� �������������, � �� �� ������ ��������.", ch);
	    return false;
	}
    }
}


