#pragma once
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#include <intrin.h>
#include <iostream>
#include "general.h"

//static U64 _rays[8][64];
static constexpr U64 _rays[8][64] = { {72340172838076672ULL, 144680345676153344ULL, 289360691352306688ULL, 578721382704613376ULL, 1157442765409226752ULL, 2314885530818453504ULL, 4629771061636907008ULL, 9259542123273814016ULL, 72340172838076416ULL, 144680345676152832ULL, 289360691352305664ULL, 578721382704611328ULL, 1157442765409222656ULL, 2314885530818445312ULL, 4629771061636890624ULL, 9259542123273781248ULL, 72340172838010880ULL, 144680345676021760ULL, 289360691352043520ULL, 578721382704087040ULL, 1157442765408174080ULL, 2314885530816348160ULL, 4629771061632696320ULL, 9259542123265392640ULL, 72340172821233664ULL, 144680345642467328ULL, 289360691284934656ULL, 578721382569869312ULL, 1157442765139738624ULL, 2314885530279477248ULL, 4629771060558954496ULL, 9259542121117908992ULL, 72340168526266368ULL, 144680337052532736ULL, 289360674105065472ULL, 578721348210130944ULL, 1157442696420261888ULL, 2314885392840523776ULL, 4629770785681047552ULL, 9259541571362095104ULL, 72339069014638592ULL, 144678138029277184ULL, 289356276058554368ULL, 578712552117108736ULL, 1157425104234217472ULL, 2314850208468434944ULL, 4629700416936869888ULL, 9259400833873739776ULL, 72057594037927936ULL, 144115188075855872ULL, 288230376151711744ULL, 576460752303423488ULL, 1152921504606846976ULL, 2305843009213693952ULL, 4611686018427387904ULL, 9223372036854775808ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}, {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 1ULL, 2ULL, 4ULL, 8ULL, 16ULL, 32ULL, 64ULL, 128ULL, 257ULL, 514ULL, 1028ULL, 2056ULL, 4112ULL, 8224ULL, 16448ULL, 32896ULL, 65793ULL, 131586ULL, 263172ULL, 526344ULL, 1052688ULL, 2105376ULL, 4210752ULL, 8421504ULL, 16843009ULL, 33686018ULL, 67372036ULL, 134744072ULL, 269488144ULL, 538976288ULL, 1077952576ULL, 2155905152ULL, 4311810305ULL, 8623620610ULL, 17247241220ULL, 34494482440ULL, 68988964880ULL, 137977929760ULL, 275955859520ULL, 551911719040ULL, 1103823438081ULL, 2207646876162ULL, 4415293752324ULL, 8830587504648ULL, 17661175009296ULL, 35322350018592ULL, 70644700037184ULL, 141289400074368ULL, 282578800148737ULL, 565157600297474ULL, 1130315200594948ULL, 2260630401189896ULL, 4521260802379792ULL, 9042521604759584ULL, 18085043209519168ULL, 36170086419038336ULL}, {254ULL, 252ULL, 248ULL, 240ULL, 224ULL, 192ULL, 128ULL, 0ULL, 65024ULL, 64512ULL, 63488ULL, 61440ULL, 57344ULL, 49152ULL, 32768ULL, 0ULL, 16646144ULL, 16515072ULL, 16252928ULL, 15728640ULL, 14680064ULL, 12582912ULL, 8388608ULL, 0ULL, 4261412864ULL, 4227858432ULL, 4160749568ULL, 4026531840ULL, 3758096384ULL, 3221225472ULL, 2147483648ULL, 0ULL, 1090921693184ULL, 1082331758592ULL, 1065151889408ULL, 1030792151040ULL, 962072674304ULL, 824633720832ULL, 549755813888ULL, 0ULL, 279275953455104ULL, 277076930199552ULL, 272678883688448ULL, 263882790666240ULL, 246290604621824ULL, 211106232532992ULL, 140737488355328ULL, 0ULL, 71494644084506624ULL, 70931694131085312ULL, 69805794224242688ULL, 67553994410557440ULL, 63050394783186944ULL, 54043195528445952ULL, 36028797018963968ULL, 0ULL, 18302628885633695744ULL, 18158513697557839872ULL, 17870283321406128128ULL, 17293822569102704640ULL, 16140901064495857664ULL, 13835058055282163712ULL, 9223372036854775808ULL, 0ULL}, {0ULL, 1ULL, 3ULL, 7ULL, 15ULL, 31ULL, 63ULL, 127ULL, 0ULL, 256ULL, 768ULL, 1792ULL, 3840ULL, 7936ULL, 16128ULL, 32512ULL, 0ULL, 65536ULL, 196608ULL, 458752ULL, 983040ULL, 2031616ULL, 4128768ULL, 8323072ULL, 0ULL, 16777216ULL, 50331648ULL, 117440512ULL, 251658240ULL, 520093696ULL, 1056964608ULL, 2130706432ULL, 0ULL, 4294967296ULL, 12884901888ULL, 30064771072ULL, 64424509440ULL, 133143986176ULL, 270582939648ULL, 545460846592ULL, 0ULL, 1099511627776ULL, 3298534883328ULL, 7696581394432ULL, 16492674416640ULL, 34084860461056ULL, 69269232549888ULL, 139637976727552ULL, 0ULL, 281474976710656ULL, 844424930131968ULL, 1970324836974592ULL, 4222124650659840ULL, 8725724278030336ULL, 17732923532771328ULL, 35747322042253312ULL, 0ULL, 72057594037927936ULL, 216172782113783808ULL, 504403158265495552ULL, 1080863910568919040ULL, 2233785415175766016ULL, 4539628424389459968ULL, 9151314442816847872ULL}, {9241421688590303744ULL, 36099303471055872ULL, 141012904183808ULL, 550831656960ULL, 2151686144ULL, 8404992ULL, 32768ULL, 0ULL, 4620710844295151616ULL, 9241421688590303232ULL, 36099303471054848ULL, 141012904181760ULL, 550831652864ULL, 2151677952ULL, 8388608ULL, 0ULL, 2310355422147510272ULL, 4620710844295020544ULL, 9241421688590041088ULL, 36099303470530560ULL, 141012903133184ULL, 550829555712ULL, 2147483648ULL, 0ULL, 1155177711056977920ULL, 2310355422113955840ULL, 4620710844227911680ULL, 9241421688455823360ULL, 36099303202095104ULL, 141012366262272ULL, 549755813888ULL, 0ULL, 577588851233521664ULL, 1155177702467043328ULL, 2310355404934086656ULL, 4620710809868173312ULL, 9241421619736346624ULL, 36099165763141632ULL, 140737488355328ULL, 0ULL, 288793326105133056ULL, 577586652210266112ULL, 1155173304420532224ULL, 2310346608841064448ULL, 4620693217682128896ULL, 9241386435364257792ULL, 36028797018963968ULL, 0ULL, 144115188075855872ULL, 288230376151711744ULL, 576460752303423488ULL, 1152921504606846976ULL, 2305843009213693952ULL, 4611686018427387904ULL, 9223372036854775808ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}, {0ULL, 256ULL, 66048ULL, 16909312ULL, 4328785920ULL, 1108169199616ULL, 283691315109888ULL, 72624976668147712ULL, 0ULL, 65536ULL, 16908288ULL, 4328783872ULL, 1108169195520ULL, 283691315101696ULL, 72624976668131328ULL, 145249953336262656ULL, 0ULL, 16777216ULL, 4328521728ULL, 1108168671232ULL, 283691314053120ULL, 72624976666034176ULL, 145249953332068352ULL, 290499906664136704ULL, 0ULL, 4294967296ULL, 1108101562368ULL, 283691179835392ULL, 72624976397598720ULL, 145249952795197440ULL, 290499905590394880ULL, 580999811180789760ULL, 0ULL, 1099511627776ULL, 283673999966208ULL, 72624942037860352ULL, 145249884075720704ULL, 290499768151441408ULL, 580999536302882816ULL, 1161999072605765632ULL, 0ULL, 281474976710656ULL, 72620543991349248ULL, 145241087982698496ULL, 290482175965396992ULL, 580964351930793984ULL, 1161928703861587968ULL, 2323857407723175936ULL, 0ULL, 72057594037927936ULL, 144115188075855872ULL, 288230376151711744ULL, 576460752303423488ULL, 1152921504606846976ULL, 2305843009213693952ULL, 4611686018427387904ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}, {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 2ULL, 4ULL, 8ULL, 16ULL, 32ULL, 64ULL, 128ULL, 0ULL, 516ULL, 1032ULL, 2064ULL, 4128ULL, 8256ULL, 16512ULL, 32768ULL, 0ULL, 132104ULL, 264208ULL, 528416ULL, 1056832ULL, 2113664ULL, 4227072ULL, 8388608ULL, 0ULL, 33818640ULL, 67637280ULL, 135274560ULL, 270549120ULL, 541097984ULL, 1082130432ULL, 2147483648ULL, 0ULL, 8657571872ULL, 17315143744ULL, 34630287488ULL, 69260574720ULL, 138521083904ULL, 277025390592ULL, 549755813888ULL, 0ULL, 2216338399296ULL, 4432676798592ULL, 8865353596928ULL, 17730707128320ULL, 35461397479424ULL, 70918499991552ULL, 140737488355328ULL, 0ULL, 567382630219904ULL, 1134765260439552ULL, 2269530520813568ULL, 4539061024849920ULL, 9078117754732544ULL, 18155135997837312ULL, 36028797018963968ULL, 0ULL}, {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 1ULL, 2ULL, 4ULL, 8ULL, 16ULL, 32ULL, 64ULL, 0ULL, 256ULL, 513ULL, 1026ULL, 2052ULL, 4104ULL, 8208ULL, 16416ULL, 0ULL, 65536ULL, 131328ULL, 262657ULL, 525314ULL, 1050628ULL, 2101256ULL, 4202512ULL, 0ULL, 16777216ULL, 33619968ULL, 67240192ULL, 134480385ULL, 268960770ULL, 537921540ULL, 1075843080ULL, 0ULL, 4294967296ULL, 8606711808ULL, 17213489152ULL, 34426978560ULL, 68853957121ULL, 137707914242ULL, 275415828484ULL, 0ULL, 1099511627776ULL, 2203318222848ULL, 4406653222912ULL, 8813306511360ULL, 17626613022976ULL, 35253226045953ULL, 70506452091906ULL, 0ULL, 281474976710656ULL, 564049465049088ULL, 1128103225065472ULL, 2256206466908160ULL, 4512412933881856ULL, 9024825867763968ULL, 18049651735527937ULL} };
static constexpr U64 not_a_file = 0xfefefefefefefefe;
static constexpr U64 not_h_file = 0x7f7f7f7f7f7f7f7f;

enum Dir {
    NORTH,
    SOUTH,
    EAST,
    WEST,
    NORTH_EAST,
    NORTH_WEST,
    SOUTH_EAST,
    SOUTH_WEST
};

int _row(int sq) {
    return sq / 8;
}

int _col(int sq) {
    return sq % 8;
}

U64 _eastN(U64 mask, int n) {
    U64 newmask = mask;
    for (int i = 0; i < n; i++) {
        newmask = ((newmask << 1) & (not_a_file));
    }

    return newmask;
}

U64 _westN(U64 mask, int n) {
    U64 newmask = mask;
    for (int i = 0; i < n; i++) {
        newmask = ((newmask >> 1) & (not_h_file));
    }

    return newmask;
}

//void init_rays() {
//    for (int sq = 0; sq < 64; sq++) {
//        // North
//        _rays[NORTH][sq] = 0x0101010101010100ULL << sq;
//
//        // South
//        _rays[SOUTH][sq] = 0x0080808080808080ULL >> (63 - sq);
//
//        // East
//        _rays[EAST][sq] = 2 * ((1ULL << (sq | 7)) - (1ULL << sq));
//
//        // West
//        _rays[WEST][sq] = (1ULL << sq) - (1ULL << (sq & 56));
//
//        // North West
//        _rays[NORTH_WEST][sq] = _westN(0x102040810204000ULL, 7 - _col(sq)) << (_row(sq) * 8);
//
//        // North East
//        _rays[NORTH_EAST][sq] = _eastN(0x8040201008040200ULL, _col(sq)) << (_row(sq) * 8);
//
//        // South West
//        _rays[SOUTH_WEST][sq] = _westN(0x40201008040201ULL, 7 - _col(sq)) >> ((7 - _row(sq)) * 8);
//
//        // South East
//        _rays[SOUTH_EAST][sq] = _eastN(0x2040810204080ULL, _col(sq)) >> ((7 - _row(sq)) * 8);
//    }
//}