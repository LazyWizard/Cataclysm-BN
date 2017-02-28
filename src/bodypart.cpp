#include "bodypart.h"
#include "anatomy.h"
#include "translations.h"
#include "rng.h"
#include "debug.h"
#include "generic_factory.h"
#include <map>
#include <unordered_map>

side opposite_side( side s )
{
    switch( s ) {
        case side::BOTH:
            return side::BOTH;
        case side::LEFT:
            return side::RIGHT;
        case side::RIGHT:
            return side::LEFT;
    }

    return s;
}

namespace io
{

static const std::map<std::string, side> side_map = {{
        { "left", side::LEFT },
        { "right", side::RIGHT },
        { "both", side::BOTH }
    }
};

template<>
side string_to_enum<side>( const std::string &data )
{
    return string_to_enum_look_up( side_map, data );
}

}

namespace
{

generic_factory<body_part_struct> body_part_factory( "body part" );

} // namespace

body_part legacy_id_to_enum( const std::string &legacy_id )
{
    static const std::unordered_map<std::string, body_part> body_parts = {
        { "TORSO", bp_torso },
        { "HEAD", bp_head },
        { "EYES", bp_eyes },
        { "MOUTH", bp_mouth },
        { "ARM_L", bp_arm_l },
        { "ARM_R", bp_arm_r },
        { "HAND_L", bp_hand_l },
        { "HAND_R", bp_hand_r },
        { "LEG_L", bp_leg_l },
        { "LEG_R", bp_leg_r },
        { "FOOT_L", bp_foot_l },
        { "FOOT_R", bp_foot_r },
        { "NUM_BP", num_bp },
    };
    const auto &iter = body_parts.find( legacy_id );
    if( iter == body_parts.end() ) {
        debugmsg( "Invalid body part legacy id %s", legacy_id.c_str() );
        return num_bp;
    }

    return iter->second;
}

template<>
const bodypart_ids bodypart_ids::NULL_ID( "num_bp" );

template<>
bool bodypart_ids::is_valid() const
{
    return body_part_factory.is_valid( *this );
}

template<>
bool bodypart_id::is_valid() const
{
    return body_part_factory.is_valid( *this );
}

template<>
const body_part_struct &bodypart_ids::obj() const
{
    return body_part_factory.obj( *this );
}

template<>
const body_part_struct &bodypart_id::obj() const
{
    return body_part_factory.obj( *this );
}

template<>
const bodypart_ids &bodypart_id::id() const
{
    return body_part_factory.convert( *this );
}

template<>
bodypart_id bodypart_ids::id() const
{
    return body_part_factory.convert( *this, bodypart_id( 0 ) );
}

body_part get_body_part_token( const std::string &id )
{
    return legacy_id_to_enum( id );
}

const bodypart_ids &convert_bp( body_part token )
{
    static const std::unordered_map<body_part, bodypart_ids> body_parts = {
        { bp_torso, bodypart_ids( "torso" ) },
        { bp_head, bodypart_ids( "head" ) },
        { bp_eyes, bodypart_ids( "eyes" ) },
        { bp_mouth, bodypart_ids( "mouth" ) },
        { bp_arm_l, bodypart_ids( "arm_l" ) },
        { bp_arm_r, bodypart_ids( "arm_r" ) },
        { bp_hand_l, bodypart_ids( "hand_l" ) },
        { bp_hand_r, bodypart_ids( "hand_r" ) },
        { bp_leg_l, bodypart_ids( "leg_l" ) },
        { bp_leg_r, bodypart_ids( "leg_r" ) },
        { bp_foot_l, bodypart_ids( "foot_l" ) },
        { bp_foot_r, bodypart_ids( "foot_r" ) },
        { num_bp, bodypart_ids( "num_bp" ) },
    };
    const auto &iter = body_parts.find( token );
    if( iter == body_parts.end() ) {
        debugmsg( "Invalid body part token %d", token );
        return body_parts.find( num_bp )->second;
    }

    return iter->second;
}

const body_part_struct &get_bp( body_part bp )
{
    return convert_bp( bp ).obj();
}

void body_part_struct::load_bp( JsonObject &jo, const std::string &src )
{
    body_part_factory.load( jo, src );
}

void body_part_struct::load( JsonObject &jo, const std::string & )
{
    mandatory( jo, was_loaded, "id", id );

    mandatory( jo, was_loaded, "name", name );
    mandatory( jo, was_loaded, "heading_singular", name_as_heading_singular );
    mandatory( jo, was_loaded, "heading_plural", name_as_heading_multiple );
    mandatory( jo, was_loaded, "encumbrance_text", encumb_text );
    mandatory( jo, was_loaded, "hit_size", hit_size );
    //mandatory( jo, was_loaded, "hit_size_relative", hit_size_relative );

    mandatory( jo, was_loaded, "legacy_id", legacy_id );
    token = legacy_id_to_enum( legacy_id );

    mandatory( jo, was_loaded, "main_part", main_part );
    mandatory( jo, was_loaded, "opposite_part", opposite_part );

    part_side = jo.get_enum_value<side>( "side" );
}

void body_part_struct::reset()
{
    body_part_factory.reset();
}

void body_part_struct::finalize_all()
{
    body_part_factory.finalize();
}

void body_part_struct::finalize()
{
}

void body_part_struct::check_consistency()
{
    for( size_t i = 0; i < num_bp; i++ ) {
        const auto &legacy_bp = convert_bp( static_cast<body_part>( i ) );
        if( !legacy_bp.is_valid() ) {
            debugmsg( "Mandatory body part %s was not loaded", legacy_bp.c_str() );
        }
    }

    body_part_factory.check();
}

void body_part_struct::check() const
{
    const auto &under_token = get_bp( token );
    if( this != &under_token ) {
        debugmsg( "Body part %s has duplicate token %d, mapped to %d", id.c_str(), token, under_token.id.c_str() );
    }

    if( id != NULL_ID && main_part == NULL_ID ) {
        debugmsg( "Body part %s has unset main part", id.c_str() );
    }

    if( id != NULL_ID && opposite_part == NULL_ID ) {
        debugmsg( "Body part %s has unset opposite part", id.c_str() );
    }

    if( !main_part.is_valid() ) {
        debugmsg( "Body part %s has invalid main part %s.", id.c_str(), main_part.c_str() );
    }

    if( !opposite_part.is_valid() ) {
        debugmsg( "Body part %s has invalid opposite part %s.", id.c_str(), opposite_part.c_str() );
    }
}

std::string body_part_name( body_part bp )
{
    return _( get_bp( bp ).name.c_str() );
}

std::string body_part_name_accusative( body_part bp )
{
    return pgettext( "bodypart_accusative", get_bp( bp ).name.c_str() );
}

std::string body_part_name_as_heading( body_part bp, int number )
{
    const auto &bdy = get_bp( bp );
    return ngettext( bdy.name_as_heading_singular.c_str(), bdy.name_as_heading_multiple.c_str(),
                     number );
}

std::string encumb_text( body_part bp )
{
    const std::string &txt = get_bp( bp ).encumb_text;
    return !txt.empty() ? _( txt.c_str() ) : txt;
}

body_part random_body_part( bool main_parts_only )
{
    const auto &part = human_anatomy->random_body_part();
    return main_parts_only ? part->main_part->token : part->token;
}

body_part mutate_to_main_part( body_part bp )
{
    return get_bp( bp ).main_part->token;
}

body_part opposite_body_part( body_part bp )
{
    return get_bp( bp ).opposite_part->token;
}

std::string get_body_part_id( body_part bp )
{
    return get_bp( bp ).legacy_id;
}
