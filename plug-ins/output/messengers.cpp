#include "json/json.h"
#include "logstream.h"
#include "messengers.h"
#include "iconvmap.h"
#include "dlfileop.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "dreamland.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

static IconvMap koi2utf("koi8-r", "utf-8");


void send_telegram(const DLString &content)
{
    try {
        Json::Value body;
        body["chat_id"] = "@dreamland_rocks";
        body["parse_mode"] = "Markdown";
        body["text"] = koi2utf(content.colourStrip());
        
        Json::FastWriter writer;
        DLDirectory dir( dreamland->getMiscDir( ), "telegram" );
        DLFileStream( dir.tempEntry( ) ).fromString( 
            writer.write(body)
        );

    } catch (const Exception &e) {
        LogStream::sendError() << "Send to telegram: " << e.what() << endl;
    }
}

static DLString discord_string(const DLString &s)
{
    DLString utf = koi2utf(s.colourStrip());
    if (utf.length() > 2000) {
        utf.cutSize(1995);
        utf << "\n...";
    }
    return utf;
}

static void send_discord(Json::Value &body)
{
    try {

        DLDirectory dir( dreamland->getMiscDir( ), "discord" );

        Json::FastWriter writer;
        DLFileStream( dir.tempEntry( ) ).fromString( 
            writer.write(body)
        );

    } catch (const Exception &e) {
        LogStream::sendError() << "Send to discord: " << e.what() << endl;
    }
}

/**
 * Send out-of-character channel messages with real usernames.
 */
void send_discord_ooc(Character *ch, const DLString &format, const DLString &msg)
{
    DLString description = fmt(0, format.c_str(), ch, msg.c_str(), 0);

    Json::Value body;
    body["username"] = koi2utf("Каналы");
    body["content"] = koi2utf(description.colourStrip());

    send_discord(body);
}

/** 
 * Send in-char channel message, with speaker name seen from p.o.v. of someone
 * w/o any detects. 
 */
void send_discord_ic(Character *ch, const DLString &format, const DLString &msg)
{
    // Create a pseudo-player, with just enough parameters in order not to crash.
    PCharacter vict;
    vict.in_room = get_room_index(2);
    vict.config.setBit(CONFIG_RUNAMES);

    DLString description = fmt(&vict, format.c_str(), ch, msg.c_str(), 0);

    Json::Value body;
    body["username"] = koi2utf("Каналы");
    body["content"] = koi2utf(description.colourStrip());

    send_discord(body);
}

static const DLString COLOR_PINK = "14132165";
static const DLString COLOR_CYAN = "2088924";
static const DLString COLOR_GOLDEN = "16640598";
static const DLString COLOR_GREEN = "4485139";
static const DLString COLOR_CRIMSON = "14824462";
static const DLString COLOR_BLUE = "4514034";

static const DLString ORB_USERNAME = "Хрустальный шар";

void send_discord_orb(const DLString &msg)
{
    Json::Value body;
    body["username"] = koi2utf(ORB_USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg.colourStrip());
    body["embeds"][0]["color"] = COLOR_PINK;

    send_discord(body);
}

void send_discord_clan(const DLString &msg)
{
    static const DLString USERNAME = "Кланы";

    Json::Value body;
    body["username"] = koi2utf(USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg.colourStrip());
    body["embeds"][0]["color"] = COLOR_CYAN;

    send_discord(body);
}

void send_discord_gquest(const DLString &gqName, const DLString &msg)
{
    static const DLString USERNAME = "Глобальные квесты";

    Json::Value body;
    body["username"] = koi2utf(USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg.colourStrip());
    body["embeds"][0]["color"] = COLOR_GOLDEN;
    body["embeds"][0]["author"]["name"] = koi2utf(gqName);

    send_discord(body);
}

// TODO 
void send_discord_level(PCharacter *ch)
{   
    Json::Value body;
    DLString msg;

    if (ch->getLevel() == LEVEL_MORTAL)
        msg = fmt(0, "%1$^C1 достиг%1$Gло||ла уровня героя!", ch);
    else
        msg = fmt(0, "%1$^C1 достиг%1$Gло||ла следующей ступени мастерства.", ch);

    body["username"] = koi2utf(ORB_USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg);
    body["embeds"][0]["color"] = COLOR_PINK;

    send_discord(body);
}

void send_discord_bonus(const DLString &msg)
{
    static const DLString USERNAME = "Календарь";

    Json::Value body;
    body["username"] = koi2utf(USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg.colourStrip());
    body["embeds"][0]["color"] = COLOR_GREEN;

    send_discord(body);
}

void send_discord_death(PCharacter *ch, Character *killer)
{
    DLString msg;
    if (!killer || killer == ch)
        msg = fmt(0, "%1$C1 погиб%1$Gло||ла своей смертью.", ch);
    else
        msg = fmt(0, "%1$C1 па%1$Gло|л|ла от руки %2$C2.", ch, killer);

    Json::Value body;
    body["username"] = koi2utf(ORB_USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg);
    body["embeds"][0]["color"] = COLOR_CRIMSON;

    send_discord(body);
}


void send_discord_news(const DLString &author, const DLString &title, const DLString &description)
{
    static const DLString USERNAME = "Новости и изменения";

    Json::Value body;
    body["username"] = koi2utf(USERNAME);
    body["embeds"][0]["title"] = koi2utf(title.colourStrip());
    body["embeds"][0]["description"] = discord_string(description);
    body["embeds"][0]["url"] = "https://dreamland.rocks/news.html";
    body["embeds"][0]["color"] = COLOR_BLUE;
    body["embeds"][0]["author"]["name"] = koi2utf(author.colourStrip());

    send_discord(body);
}
