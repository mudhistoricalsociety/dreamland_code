/* $Id$
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "xmlattributetrust.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "skillreference.h"

#include "loadsave.h"
#include "move_utils.h"
#include "occupations.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(riding);

/*
 * 'mount' command
 */
CMDRUN( mount )
{
    XMLAttributeTrust::Pointer trust;
    Character *horse;
    int moveCost = 1;
    DLString arg, args = constArguments;

    if (IS_SET( ch->form, FORM_CENTAUR )) {
        ostringstream buf;
        
        if (ch->is_npc( ) || IS_CHARMED(ch)) {
            ch->println( "Да ты и так на коне." );
            return;
        }
        
        trust = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeTrust>( "mount" );

        if (trust->parse( constArguments, buf )) 
            ch->send_to( "Ездить на тебе верхом " );
            
        ch->println( buf.str( ) );
        return;
    }
    
    if (MOUNTED(ch)) {
        ch->println( "Ты уже верхом." );
        return;
    }

    if (RIDDEN(ch)) {
        ch->pecho( "Но тебя сам%Gо|ого|у оседали!", ch );
        return;
    }

    arg = args.getOneArgument( );

    if (( horse = get_char_room( ch, arg.c_str( ) ) ) == NULL) {
        ch->println( "Кого ты хочешь оседлать?" );
        return;
    }
    
    if (MOUNTED(horse) || horse == ch) {
        ch->pecho( "Мы не в цирке!" );
        return;
    }
    
    if (RIDDEN(horse)) {
        ch->pecho( "%1$^C1 уже оседла%1$Gно|н|на.", horse );
        return;
    }
    
    if (!horse->is_npc( )) { /* pc-mounts like centaurs */
        if (!IS_SET(horse->form, FORM_CENTAUR)) {
            act("$c1 пытается запрыгнуть верхом на $C4.", ch, 0, horse, TO_NOTVICT);
            act("$c1 пытается запрыгнуть верхом на тебя.", ch, 0, horse, TO_VICT);
            act("Ты пытаешься оседлать $C4, но никак не поймешь, где же у $X стремена..", ch, 0, horse, TO_CHAR);
            return;
        }
        
        trust = horse->getPC( )->getAttributes( ).findAttr<XMLAttributeTrust>( "mount" );
        if (!trust || !trust->check( ch )) {
            act("$c1 пытается оседлать $C4. $C1 строго смотрит на $c4.", ch, 0, horse, TO_NOTVICT);
            act("$c1 пытается оседлать тебя, но заметив твой строгий взгляд, останавливается.", ch, 0, horse, TO_VICT);
            act("$C1 не желает, чтобы на $Z катались.", ch, 0, horse, TO_CHAR);
            return;
        }
    }
    else { /* other rideable beasts */
        if (!IS_SET(horse->act, ACT_RIDEABLE)) {
            act("$c1 пытается запрыгнуть верхом на $C4, но соскальзывает.", ch, 0, horse, TO_NOTVICT);
            ch->println("Этот вид живых существ не предназначен для верховой езды.");
            return;
        }
        
        if (horse->getModifyLevel( ) - ch->getModifyLevel( ) > 5) {
            act("$c1 пытается оседлать $C4, но опыта явно не хватает.", ch, 0, horse, TO_NOTVICT);
            ch->println("Тебе не хватит опыта справиться с этим скакуном.");
            return;
        }
    }
    
    if ((horse->is_npc( ) || IS_CHARMED(horse))
        && horse->master 
        && horse->master != ch) 
    {
        ch->pecho("У %C2 уже есть хозяин, и это явно - не ты!", horse );
        return;
    }
    
    if (horse->position < POS_STANDING) {
        ch->pecho("%1$^C1 долж%1$Gно|ен|на для начала встать на ноги.", horse );
        return;
    }

    if (ch->move < moveCost) {
        ch->println("У тебя не хватает сил даже задрать ногу.");
        return;
    }

    // Knight horses are the only ones requiring special handling skill ('riding'). 
    bool needsRidingSkill = horse->is_npc() 
            && horse->getNPC()->behavior 
            && IS_SET(horse->getNPC()->behavior->getOccupation(), (1<<OCC_BATTLEHORSE));
   
    if (needsRidingSkill && number_percent( ) > gsn_riding->getEffective( ch )) {
        act( "Тебе не хватило мастерства оседлать $C4.", ch, 0, horse, TO_CHAR );
        act( "$c1 пытается оседлать тебя, но мастерства явно не хватает.", ch, 0, horse, TO_VICT );
        act( "$c1 пытается оседлать $C4, но мастерства явно не хватает.", ch, 0, horse, TO_NOTVICT );
        
        ch->setWait( gsn_riding->getBeats( ) );
        gsn_riding->improve( ch, false );
        return;
    }

    ch->mount = horse;
    ch->riding = true;
    horse->mount = ch;
    horse->riding = false;

    act( "Ты запрыгиваешь на $C4.", ch, 0, horse, TO_CHAR );
    act( "$c1 запрыгивает тебе на спину.", ch, 0, horse, TO_VICT );
    act( "$c1 запрыгивает на спину $C2.", ch, 0, horse, TO_NOTVICT );
    
    gsn_riding->improve( ch, true);
}

/*
 * 'dismount' command
 */
CMDRUN( dismount )
{
    /*
     * jump off the horse 
     */
    if (!ch->mount) {
        ch->println( "Ни над тобой, ни под тобой никого нет!" );
        return;
    }
    
    if (MOUNTED(ch)) {
        act( "Ты соскакиваешь со спины $C2.", ch, 0, ch->mount, TO_CHAR );
        act( "$c1 соскакивает с твоей спины.", ch, 0, ch->mount, TO_VICT );
        act( "$c1 спрыгивает с $C2.", ch, 0, ch->mount, TO_NOTVICT );
    }
    else {
        act( "Ты сбрасываешь $C4 со спины.", ch, 0, ch->mount, TO_CHAR );
        act( "$c1 сбрасывает тебя со спины.", ch, 0, ch->mount, TO_VICT );
        act( "$c1 сбрасывает $C4 со спины.", ch, 0, ch->mount, TO_NOTVICT );
    }
    
    ch->dismount( );
}

