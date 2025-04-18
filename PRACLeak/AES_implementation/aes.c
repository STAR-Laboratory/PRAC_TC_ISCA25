#include<stdint.h>
#include<stdio.h>
#include<time.h>

typedef unsigned int u32;
typedef uint8_t u8;

#define KEY_LENGTH 128

u32 invT0[256], invT1[256], invT2[256], invT3[256];

const unsigned int Te0[256] = {
	0xc66363a5U, 0xf87c7c84U, 0xee777799U, 0xf67b7b8dU,
	0xfff2f20dU, 0xd66b6bbdU, 0xde6f6fb1U, 0x91c5c554U,
	0x60303050U, 0x02010103U, 0xce6767a9U, 0x562b2b7dU,
	0xe7fefe19U, 0xb5d7d762U, 0x4dababe6U, 0xec76769aU,
	0x8fcaca45U, 0x1f82829dU, 0x89c9c940U, 0xfa7d7d87U,
	0xeffafa15U, 0xb25959ebU, 0x8e4747c9U, 0xfbf0f00bU,
	0x41adadecU, 0xb3d4d467U, 0x5fa2a2fdU, 0x45afafeaU,
	0x239c9cbfU, 0x53a4a4f7U, 0xe4727296U, 0x9bc0c05bU,
	0x75b7b7c2U, 0xe1fdfd1cU, 0x3d9393aeU, 0x4c26266aU,
	0x6c36365aU, 0x7e3f3f41U, 0xf5f7f702U, 0x83cccc4fU,
	0x6834345cU, 0x51a5a5f4U, 0xd1e5e534U, 0xf9f1f108U,
	0xe2717193U, 0xabd8d873U, 0x62313153U, 0x2a15153fU,
	0x0804040cU, 0x95c7c752U, 0x46232365U, 0x9dc3c35eU,
	0x30181828U, 0x379696a1U, 0x0a05050fU, 0x2f9a9ab5U,
	0x0e070709U, 0x24121236U, 0x1b80809bU, 0xdfe2e23dU,
	0xcdebeb26U, 0x4e272769U, 0x7fb2b2cdU, 0xea75759fU,
	0x1209091bU, 0x1d83839eU, 0x582c2c74U, 0x341a1a2eU,
	0x361b1b2dU, 0xdc6e6eb2U, 0xb45a5aeeU, 0x5ba0a0fbU,
	0xa45252f6U, 0x763b3b4dU, 0xb7d6d661U, 0x7db3b3ceU,
	0x5229297bU, 0xdde3e33eU, 0x5e2f2f71U, 0x13848497U,
	0xa65353f5U, 0xb9d1d168U, 0x00000000U, 0xc1eded2cU,
	0x40202060U, 0xe3fcfc1fU, 0x79b1b1c8U, 0xb65b5bedU,
	0xd46a6abeU, 0x8dcbcb46U, 0x67bebed9U, 0x7239394bU,
	0x944a4adeU, 0x984c4cd4U, 0xb05858e8U, 0x85cfcf4aU,
	0xbbd0d06bU, 0xc5efef2aU, 0x4faaaae5U, 0xedfbfb16U,
	0x864343c5U, 0x9a4d4dd7U, 0x66333355U, 0x11858594U,
	0x8a4545cfU, 0xe9f9f910U, 0x04020206U, 0xfe7f7f81U,
	0xa05050f0U, 0x783c3c44U, 0x259f9fbaU, 0x4ba8a8e3U,
	0xa25151f3U, 0x5da3a3feU, 0x804040c0U, 0x058f8f8aU,
	0x3f9292adU, 0x219d9dbcU, 0x70383848U, 0xf1f5f504U,
	0x63bcbcdfU, 0x77b6b6c1U, 0xafdada75U, 0x42212163U,
	0x20101030U, 0xe5ffff1aU, 0xfdf3f30eU, 0xbfd2d26dU,
	0x81cdcd4cU, 0x180c0c14U, 0x26131335U, 0xc3ecec2fU,
	0xbe5f5fe1U, 0x359797a2U, 0x884444ccU, 0x2e171739U,
	0x93c4c457U, 0x55a7a7f2U, 0xfc7e7e82U, 0x7a3d3d47U,
	0xc86464acU, 0xba5d5de7U, 0x3219192bU, 0xe6737395U,
	0xc06060a0U, 0x19818198U, 0x9e4f4fd1U, 0xa3dcdc7fU,
	0x44222266U, 0x542a2a7eU, 0x3b9090abU, 0x0b888883U,
	0x8c4646caU, 0xc7eeee29U, 0x6bb8b8d3U, 0x2814143cU,
	0xa7dede79U, 0xbc5e5ee2U, 0x160b0b1dU, 0xaddbdb76U,
	0xdbe0e03bU, 0x64323256U, 0x743a3a4eU, 0x140a0a1eU,
	0x924949dbU, 0x0c06060aU, 0x4824246cU, 0xb85c5ce4U,
	0x9fc2c25dU, 0xbdd3d36eU, 0x43acacefU, 0xc46262a6U,
	0x399191a8U, 0x319595a4U, 0xd3e4e437U, 0xf279798bU,
	0xd5e7e732U, 0x8bc8c843U, 0x6e373759U, 0xda6d6db7U,
	0x018d8d8cU, 0xb1d5d564U, 0x9c4e4ed2U, 0x49a9a9e0U,
	0xd86c6cb4U, 0xac5656faU, 0xf3f4f407U, 0xcfeaea25U,
	0xca6565afU, 0xf47a7a8eU, 0x47aeaee9U, 0x10080818U,
	0x6fbabad5U, 0xf0787888U, 0x4a25256fU, 0x5c2e2e72U,
	0x381c1c24U, 0x57a6a6f1U, 0x73b4b4c7U, 0x97c6c651U,
	0xcbe8e823U, 0xa1dddd7cU, 0xe874749cU, 0x3e1f1f21U,
	0x964b4bddU, 0x61bdbddcU, 0x0d8b8b86U, 0x0f8a8a85U,
	0xe0707090U, 0x7c3e3e42U, 0x71b5b5c4U, 0xcc6666aaU,
	0x904848d8U, 0x06030305U, 0xf7f6f601U, 0x1c0e0e12U,
	0xc26161a3U, 0x6a35355fU, 0xae5757f9U, 0x69b9b9d0U,
	0x17868691U, 0x99c1c158U, 0x3a1d1d27U, 0x279e9eb9U,
	0xd9e1e138U, 0xebf8f813U, 0x2b9898b3U, 0x22111133U,
	0xd26969bbU, 0xa9d9d970U, 0x078e8e89U, 0x339494a7U,
	0x2d9b9bb6U, 0x3c1e1e22U, 0x15878792U, 0xc9e9e920U,
	0x87cece49U, 0xaa5555ffU, 0x50282878U, 0xa5dfdf7aU,
	0x038c8c8fU, 0x59a1a1f8U, 0x09898980U, 0x1a0d0d17U,
	0x65bfbfdaU, 0xd7e6e631U, 0x844242c6U, 0xd06868b8U,
	0x824141c3U, 0x299999b0U, 0x5a2d2d77U, 0x1e0f0f11U,
	0x7bb0b0cbU, 0xa85454fcU, 0x6dbbbbd6U, 0x2c16163aU,
};
const unsigned int Te1[256] = {
	0xa5c66363U, 0x84f87c7cU, 0x99ee7777U, 0x8df67b7bU,
	0x0dfff2f2U, 0xbdd66b6bU, 0xb1de6f6fU, 0x5491c5c5U,
	0x50603030U, 0x03020101U, 0xa9ce6767U, 0x7d562b2bU,
	0x19e7fefeU, 0x62b5d7d7U, 0xe64dababU, 0x9aec7676U,
	0x458fcacaU, 0x9d1f8282U, 0x4089c9c9U, 0x87fa7d7dU,
	0x15effafaU, 0xebb25959U, 0xc98e4747U, 0x0bfbf0f0U,
	0xec41adadU, 0x67b3d4d4U, 0xfd5fa2a2U, 0xea45afafU,
	0xbf239c9cU, 0xf753a4a4U, 0x96e47272U, 0x5b9bc0c0U,
	0xc275b7b7U, 0x1ce1fdfdU, 0xae3d9393U, 0x6a4c2626U,
	0x5a6c3636U, 0x417e3f3fU, 0x02f5f7f7U, 0x4f83ccccU,
	0x5c683434U, 0xf451a5a5U, 0x34d1e5e5U, 0x08f9f1f1U,
	0x93e27171U, 0x73abd8d8U, 0x53623131U, 0x3f2a1515U,
	0x0c080404U, 0x5295c7c7U, 0x65462323U, 0x5e9dc3c3U,
	0x28301818U, 0xa1379696U, 0x0f0a0505U, 0xb52f9a9aU,
	0x090e0707U, 0x36241212U, 0x9b1b8080U, 0x3ddfe2e2U,
	0x26cdebebU, 0x694e2727U, 0xcd7fb2b2U, 0x9fea7575U,
	0x1b120909U, 0x9e1d8383U, 0x74582c2cU, 0x2e341a1aU,
	0x2d361b1bU, 0xb2dc6e6eU, 0xeeb45a5aU, 0xfb5ba0a0U,
	0xf6a45252U, 0x4d763b3bU, 0x61b7d6d6U, 0xce7db3b3U,
	0x7b522929U, 0x3edde3e3U, 0x715e2f2fU, 0x97138484U,
	0xf5a65353U, 0x68b9d1d1U, 0x00000000U, 0x2cc1ededU,
	0x60402020U, 0x1fe3fcfcU, 0xc879b1b1U, 0xedb65b5bU,
	0xbed46a6aU, 0x468dcbcbU, 0xd967bebeU, 0x4b723939U,
	0xde944a4aU, 0xd4984c4cU, 0xe8b05858U, 0x4a85cfcfU,
	0x6bbbd0d0U, 0x2ac5efefU, 0xe54faaaaU, 0x16edfbfbU,
	0xc5864343U, 0xd79a4d4dU, 0x55663333U, 0x94118585U,
	0xcf8a4545U, 0x10e9f9f9U, 0x06040202U, 0x81fe7f7fU,
	0xf0a05050U, 0x44783c3cU, 0xba259f9fU, 0xe34ba8a8U,
	0xf3a25151U, 0xfe5da3a3U, 0xc0804040U, 0x8a058f8fU,
	0xad3f9292U, 0xbc219d9dU, 0x48703838U, 0x04f1f5f5U,
	0xdf63bcbcU, 0xc177b6b6U, 0x75afdadaU, 0x63422121U,
	0x30201010U, 0x1ae5ffffU, 0x0efdf3f3U, 0x6dbfd2d2U,
	0x4c81cdcdU, 0x14180c0cU, 0x35261313U, 0x2fc3ececU,
	0xe1be5f5fU, 0xa2359797U, 0xcc884444U, 0x392e1717U,
	0x5793c4c4U, 0xf255a7a7U, 0x82fc7e7eU, 0x477a3d3dU,
	0xacc86464U, 0xe7ba5d5dU, 0x2b321919U, 0x95e67373U,
	0xa0c06060U, 0x98198181U, 0xd19e4f4fU, 0x7fa3dcdcU,
	0x66442222U, 0x7e542a2aU, 0xab3b9090U, 0x830b8888U,
	0xca8c4646U, 0x29c7eeeeU, 0xd36bb8b8U, 0x3c281414U,
	0x79a7dedeU, 0xe2bc5e5eU, 0x1d160b0bU, 0x76addbdbU,
	0x3bdbe0e0U, 0x56643232U, 0x4e743a3aU, 0x1e140a0aU,
	0xdb924949U, 0x0a0c0606U, 0x6c482424U, 0xe4b85c5cU,
	0x5d9fc2c2U, 0x6ebdd3d3U, 0xef43acacU, 0xa6c46262U,
	0xa8399191U, 0xa4319595U, 0x37d3e4e4U, 0x8bf27979U,
	0x32d5e7e7U, 0x438bc8c8U, 0x596e3737U, 0xb7da6d6dU,
	0x8c018d8dU, 0x64b1d5d5U, 0xd29c4e4eU, 0xe049a9a9U,
	0xb4d86c6cU, 0xfaac5656U, 0x07f3f4f4U, 0x25cfeaeaU,
	0xafca6565U, 0x8ef47a7aU, 0xe947aeaeU, 0x18100808U,
	0xd56fbabaU, 0x88f07878U, 0x6f4a2525U, 0x725c2e2eU,
	0x24381c1cU, 0xf157a6a6U, 0xc773b4b4U, 0x5197c6c6U,
	0x23cbe8e8U, 0x7ca1ddddU, 0x9ce87474U, 0x213e1f1fU,
	0xdd964b4bU, 0xdc61bdbdU, 0x860d8b8bU, 0x850f8a8aU,
	0x90e07070U, 0x427c3e3eU, 0xc471b5b5U, 0xaacc6666U,
	0xd8904848U, 0x05060303U, 0x01f7f6f6U, 0x121c0e0eU,
	0xa3c26161U, 0x5f6a3535U, 0xf9ae5757U, 0xd069b9b9U,
	0x91178686U, 0x5899c1c1U, 0x273a1d1dU, 0xb9279e9eU,
	0x38d9e1e1U, 0x13ebf8f8U, 0xb32b9898U, 0x33221111U,
	0xbbd26969U, 0x70a9d9d9U, 0x89078e8eU, 0xa7339494U,
	0xb62d9b9bU, 0x223c1e1eU, 0x92158787U, 0x20c9e9e9U,
	0x4987ceceU, 0xffaa5555U, 0x78502828U, 0x7aa5dfdfU,
	0x8f038c8cU, 0xf859a1a1U, 0x80098989U, 0x171a0d0dU,
	0xda65bfbfU, 0x31d7e6e6U, 0xc6844242U, 0xb8d06868U,
	0xc3824141U, 0xb0299999U, 0x775a2d2dU, 0x111e0f0fU,
	0xcb7bb0b0U, 0xfca85454U, 0xd66dbbbbU, 0x3a2c1616U,
};
const unsigned int Te2[256] = {
	0x63a5c663U, 0x7c84f87cU, 0x7799ee77U, 0x7b8df67bU,
	0xf20dfff2U, 0x6bbdd66bU, 0x6fb1de6fU, 0xc55491c5U,
	0x30506030U, 0x01030201U, 0x67a9ce67U, 0x2b7d562bU,
	0xfe19e7feU, 0xd762b5d7U, 0xabe64dabU, 0x769aec76U,
	0xca458fcaU, 0x829d1f82U, 0xc94089c9U, 0x7d87fa7dU,
	0xfa15effaU, 0x59ebb259U, 0x47c98e47U, 0xf00bfbf0U,
	0xadec41adU, 0xd467b3d4U, 0xa2fd5fa2U, 0xafea45afU,
	0x9cbf239cU, 0xa4f753a4U, 0x7296e472U, 0xc05b9bc0U,
	0xb7c275b7U, 0xfd1ce1fdU, 0x93ae3d93U, 0x266a4c26U,
	0x365a6c36U, 0x3f417e3fU, 0xf702f5f7U, 0xcc4f83ccU,
	0x345c6834U, 0xa5f451a5U, 0xe534d1e5U, 0xf108f9f1U,
	0x7193e271U, 0xd873abd8U, 0x31536231U, 0x153f2a15U,
	0x040c0804U, 0xc75295c7U, 0x23654623U, 0xc35e9dc3U,
	0x18283018U, 0x96a13796U, 0x050f0a05U, 0x9ab52f9aU,
	0x07090e07U, 0x12362412U, 0x809b1b80U, 0xe23ddfe2U,
	0xeb26cdebU, 0x27694e27U, 0xb2cd7fb2U, 0x759fea75U,
	0x091b1209U, 0x839e1d83U, 0x2c74582cU, 0x1a2e341aU,
	0x1b2d361bU, 0x6eb2dc6eU, 0x5aeeb45aU, 0xa0fb5ba0U,
	0x52f6a452U, 0x3b4d763bU, 0xd661b7d6U, 0xb3ce7db3U,
	0x297b5229U, 0xe33edde3U, 0x2f715e2fU, 0x84971384U,
	0x53f5a653U, 0xd168b9d1U, 0x00000000U, 0xed2cc1edU,
	0x20604020U, 0xfc1fe3fcU, 0xb1c879b1U, 0x5bedb65bU,
	0x6abed46aU, 0xcb468dcbU, 0xbed967beU, 0x394b7239U,
	0x4ade944aU, 0x4cd4984cU, 0x58e8b058U, 0xcf4a85cfU,
	0xd06bbbd0U, 0xef2ac5efU, 0xaae54faaU, 0xfb16edfbU,
	0x43c58643U, 0x4dd79a4dU, 0x33556633U, 0x85941185U,
	0x45cf8a45U, 0xf910e9f9U, 0x02060402U, 0x7f81fe7fU,
	0x50f0a050U, 0x3c44783cU, 0x9fba259fU, 0xa8e34ba8U,
	0x51f3a251U, 0xa3fe5da3U, 0x40c08040U, 0x8f8a058fU,
	0x92ad3f92U, 0x9dbc219dU, 0x38487038U, 0xf504f1f5U,
	0xbcdf63bcU, 0xb6c177b6U, 0xda75afdaU, 0x21634221U,
	0x10302010U, 0xff1ae5ffU, 0xf30efdf3U, 0xd26dbfd2U,
	0xcd4c81cdU, 0x0c14180cU, 0x13352613U, 0xec2fc3ecU,
	0x5fe1be5fU, 0x97a23597U, 0x44cc8844U, 0x17392e17U,
	0xc45793c4U, 0xa7f255a7U, 0x7e82fc7eU, 0x3d477a3dU,
	0x64acc864U, 0x5de7ba5dU, 0x192b3219U, 0x7395e673U,
	0x60a0c060U, 0x81981981U, 0x4fd19e4fU, 0xdc7fa3dcU,
	0x22664422U, 0x2a7e542aU, 0x90ab3b90U, 0x88830b88U,
	0x46ca8c46U, 0xee29c7eeU, 0xb8d36bb8U, 0x143c2814U,
	0xde79a7deU, 0x5ee2bc5eU, 0x0b1d160bU, 0xdb76addbU,
	0xe03bdbe0U, 0x32566432U, 0x3a4e743aU, 0x0a1e140aU,
	0x49db9249U, 0x060a0c06U, 0x246c4824U, 0x5ce4b85cU,
	0xc25d9fc2U, 0xd36ebdd3U, 0xacef43acU, 0x62a6c462U,
	0x91a83991U, 0x95a43195U, 0xe437d3e4U, 0x798bf279U,
	0xe732d5e7U, 0xc8438bc8U, 0x37596e37U, 0x6db7da6dU,
	0x8d8c018dU, 0xd564b1d5U, 0x4ed29c4eU, 0xa9e049a9U,
	0x6cb4d86cU, 0x56faac56U, 0xf407f3f4U, 0xea25cfeaU,
	0x65afca65U, 0x7a8ef47aU, 0xaee947aeU, 0x08181008U,
	0xbad56fbaU, 0x7888f078U, 0x256f4a25U, 0x2e725c2eU,
	0x1c24381cU, 0xa6f157a6U, 0xb4c773b4U, 0xc65197c6U,
	0xe823cbe8U, 0xdd7ca1ddU, 0x749ce874U, 0x1f213e1fU,
	0x4bdd964bU, 0xbddc61bdU, 0x8b860d8bU, 0x8a850f8aU,
	0x7090e070U, 0x3e427c3eU, 0xb5c471b5U, 0x66aacc66U,
	0x48d89048U, 0x03050603U, 0xf601f7f6U, 0x0e121c0eU,
	0x61a3c261U, 0x355f6a35U, 0x57f9ae57U, 0xb9d069b9U,
	0x86911786U, 0xc15899c1U, 0x1d273a1dU, 0x9eb9279eU,
	0xe138d9e1U, 0xf813ebf8U, 0x98b32b98U, 0x11332211U,
	0x69bbd269U, 0xd970a9d9U, 0x8e89078eU, 0x94a73394U,
	0x9bb62d9bU, 0x1e223c1eU, 0x87921587U, 0xe920c9e9U,
	0xce4987ceU, 0x55ffaa55U, 0x28785028U, 0xdf7aa5dfU,
	0x8c8f038cU, 0xa1f859a1U, 0x89800989U, 0x0d171a0dU,
	0xbfda65bfU, 0xe631d7e6U, 0x42c68442U, 0x68b8d068U,
	0x41c38241U, 0x99b02999U, 0x2d775a2dU, 0x0f111e0fU,
	0xb0cb7bb0U, 0x54fca854U, 0xbbd66dbbU, 0x163a2c16U,
};
const unsigned int Te3[256] = {
	0x6363a5c6U, 0x7c7c84f8U, 0x777799eeU, 0x7b7b8df6U,
	0xf2f20dffU, 0x6b6bbdd6U, 0x6f6fb1deU, 0xc5c55491U,
	0x30305060U, 0x01010302U, 0x6767a9ceU, 0x2b2b7d56U,
	0xfefe19e7U, 0xd7d762b5U, 0xababe64dU, 0x76769aecU,
	0xcaca458fU, 0x82829d1fU, 0xc9c94089U, 0x7d7d87faU,
	0xfafa15efU, 0x5959ebb2U, 0x4747c98eU, 0xf0f00bfbU,
	0xadadec41U, 0xd4d467b3U, 0xa2a2fd5fU, 0xafafea45U,
	0x9c9cbf23U, 0xa4a4f753U, 0x727296e4U, 0xc0c05b9bU,
	0xb7b7c275U, 0xfdfd1ce1U, 0x9393ae3dU, 0x26266a4cU,
	0x36365a6cU, 0x3f3f417eU, 0xf7f702f5U, 0xcccc4f83U,
	0x34345c68U, 0xa5a5f451U, 0xe5e534d1U, 0xf1f108f9U,
	0x717193e2U, 0xd8d873abU, 0x31315362U, 0x15153f2aU,
	0x04040c08U, 0xc7c75295U, 0x23236546U, 0xc3c35e9dU,
	0x18182830U, 0x9696a137U, 0x05050f0aU, 0x9a9ab52fU,
	0x0707090eU, 0x12123624U, 0x80809b1bU, 0xe2e23ddfU,
	0xebeb26cdU, 0x2727694eU, 0xb2b2cd7fU, 0x75759feaU,
	0x09091b12U, 0x83839e1dU, 0x2c2c7458U, 0x1a1a2e34U,
	0x1b1b2d36U, 0x6e6eb2dcU, 0x5a5aeeb4U, 0xa0a0fb5bU,
	0x5252f6a4U, 0x3b3b4d76U, 0xd6d661b7U, 0xb3b3ce7dU,
	0x29297b52U, 0xe3e33eddU, 0x2f2f715eU, 0x84849713U,
	0x5353f5a6U, 0xd1d168b9U, 0x00000000U, 0xeded2cc1U,
	0x20206040U, 0xfcfc1fe3U, 0xb1b1c879U, 0x5b5bedb6U,
	0x6a6abed4U, 0xcbcb468dU, 0xbebed967U, 0x39394b72U,
	0x4a4ade94U, 0x4c4cd498U, 0x5858e8b0U, 0xcfcf4a85U,
	0xd0d06bbbU, 0xefef2ac5U, 0xaaaae54fU, 0xfbfb16edU,
	0x4343c586U, 0x4d4dd79aU, 0x33335566U, 0x85859411U,
	0x4545cf8aU, 0xf9f910e9U, 0x02020604U, 0x7f7f81feU,
	0x5050f0a0U, 0x3c3c4478U, 0x9f9fba25U, 0xa8a8e34bU,
	0x5151f3a2U, 0xa3a3fe5dU, 0x4040c080U, 0x8f8f8a05U,
	0x9292ad3fU, 0x9d9dbc21U, 0x38384870U, 0xf5f504f1U,
	0xbcbcdf63U, 0xb6b6c177U, 0xdada75afU, 0x21216342U,
	0x10103020U, 0xffff1ae5U, 0xf3f30efdU, 0xd2d26dbfU,
	0xcdcd4c81U, 0x0c0c1418U, 0x13133526U, 0xecec2fc3U,
	0x5f5fe1beU, 0x9797a235U, 0x4444cc88U, 0x1717392eU,
	0xc4c45793U, 0xa7a7f255U, 0x7e7e82fcU, 0x3d3d477aU,
	0x6464acc8U, 0x5d5de7baU, 0x19192b32U, 0x737395e6U,
	0x6060a0c0U, 0x81819819U, 0x4f4fd19eU, 0xdcdc7fa3U,
	0x22226644U, 0x2a2a7e54U, 0x9090ab3bU, 0x8888830bU,
	0x4646ca8cU, 0xeeee29c7U, 0xb8b8d36bU, 0x14143c28U,
	0xdede79a7U, 0x5e5ee2bcU, 0x0b0b1d16U, 0xdbdb76adU,
	0xe0e03bdbU, 0x32325664U, 0x3a3a4e74U, 0x0a0a1e14U,
	0x4949db92U, 0x06060a0cU, 0x24246c48U, 0x5c5ce4b8U,
	0xc2c25d9fU, 0xd3d36ebdU, 0xacacef43U, 0x6262a6c4U,
	0x9191a839U, 0x9595a431U, 0xe4e437d3U, 0x79798bf2U,
	0xe7e732d5U, 0xc8c8438bU, 0x3737596eU, 0x6d6db7daU,
	0x8d8d8c01U, 0xd5d564b1U, 0x4e4ed29cU, 0xa9a9e049U,
	0x6c6cb4d8U, 0x5656faacU, 0xf4f407f3U, 0xeaea25cfU,
	0x6565afcaU, 0x7a7a8ef4U, 0xaeaee947U, 0x08081810U,
	0xbabad56fU, 0x787888f0U, 0x25256f4aU, 0x2e2e725cU,
	0x1c1c2438U, 0xa6a6f157U, 0xb4b4c773U, 0xc6c65197U,
	0xe8e823cbU, 0xdddd7ca1U, 0x74749ce8U, 0x1f1f213eU,
	0x4b4bdd96U, 0xbdbddc61U, 0x8b8b860dU, 0x8a8a850fU,
	0x707090e0U, 0x3e3e427cU, 0xb5b5c471U, 0x6666aaccU,
	0x4848d890U, 0x03030506U, 0xf6f601f7U, 0x0e0e121cU,
	0x6161a3c2U, 0x35355f6aU, 0x5757f9aeU, 0xb9b9d069U,
	0x86869117U, 0xc1c15899U, 0x1d1d273aU, 0x9e9eb927U,
	0xe1e138d9U, 0xf8f813ebU, 0x9898b32bU, 0x11113322U,
	0x6969bbd2U, 0xd9d970a9U, 0x8e8e8907U, 0x9494a733U,
	0x9b9bb62dU, 0x1e1e223cU, 0x87879215U, 0xe9e920c9U,
	0xcece4987U, 0x5555ffaaU, 0x28287850U, 0xdfdf7aa5U,
	0x8c8c8f03U, 0xa1a1f859U, 0x89898009U, 0x0d0d171aU,
	0xbfbfda65U, 0xe6e631d7U, 0x4242c684U, 0x6868b8d0U,
	0x4141c382U, 0x9999b029U, 0x2d2d775aU, 0x0f0f111eU,
	0xb0b0cb7bU, 0x5454fca8U, 0xbbbbd66dU, 0x16163a2cU,
};

const unsigned int RCon[10] = { 0x01000000, 0x02000000, 0x04000000, 0x08000000,
0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x1B000000, 0x36000000
};

unsigned char s[256] =
{
	0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
	0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
	0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
	0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
	0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
	0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
	0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
	0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
	0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
	0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
	0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
	0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
	0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
	0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
	0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
	0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

unsigned char inv_s[256] =
{
	0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
	0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
	0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
	0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
	0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
	0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
	0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
	0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
	0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
	0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
	0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
	0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
	0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
	0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
	0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};



#if KEY_LENGTH==128
static u8 key[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee ,0xff }; //PDF Test Key

#elif KEY_LENGTH==192
u8 key[32] = { 0x0,0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd,
				0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};									  //192bit Test Key

u32 w[52];
#elif KEY_LENGTH==256
u8 key[48] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
				0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 }; //256bit Test Key
u32 w[60];
#endif
static u32 P[4] = { 0x0, 0x0, 0x0, 0x0 };
static u32 W[60] = { 0x0, };
static u32 dW[60] = { 0x0, };

u32 RotWord(u32 in) {
	u8 k = in >> 24;
	in = (in << 8) + k;
	return in;
}

u32 SubWord(u32 word) {
	u32 result;

	result = ((s[word >> 24]) << 24) | (s[(u8)(word >> 16)] << 16) | (s[(u8)(word >> 8)] << 8)
		| (s[(u8)(word)]);
	return result;
}

u32 InvSubWord(u32 word) {
	u32 result;
	result = ((inv_s[word >> 24]) << 24) | (inv_s[(u8)(word >> 16)] << 16) | (inv_s[(u8)(word >> 8)] << 8)
		| (inv_s[(u8)(word)]);
	return result;
}

void ShiftRow(u32 *in) {
	u32 out[4];
	for (int i = 0; i<4; i++)
		out[i] = (in[i % 4] & 0xFF000000) | in[(i + 1) % 4] & 0x00FF0000 | in[(i + 2) % 4] & 0x0000FF00 | in[(i + 3) % 4] & 0x000000FF;
	for (int i = 0; i < 4; i++) in[i] = out[i];
}

void InvShiftRow(u32 *in) {
	u32 out[4];
	for (int i = 0; i<4; i++)
		out[i] = (in[i % 4] & 0xFF000000) | in[(i + 3) % 4] & 0x00FF0000 | in[(i + 2) % 4] & 0x0000FF00 | in[(i + 1) % 4] & 0x000000FF;
	for (int i = 0; i < 4; i++) in[i] = out[i];
}
u8 Mul2(u8 in) {
	return ((in << 1) & 0xFF) ^ ((in & 0x80) ? 0x1b : 0);
}
u8 Mul3(u8 in) {
	return Mul2(in) ^ in;
}
u8 Mul4(u8 in) {
	return Mul2(Mul2(in));
}
u8 Mul8(u8 in) {
	return Mul4(Mul2(in));
}
u8 Mul9(u8 in) {
	return Mul8(in) ^ in;
}
u8 Mulb(u8 in) {
	return Mul8(in) ^ Mul2(in) ^ in;
}
u8 Muld(u8 in) {
	return Mul8(in) ^ Mul4(in) ^ in;
}
u8 Mule(u8 in) {
	return Mul8(in) ^ Mul4(in) ^ Mul2(in);
}

void TableGen(u32 T0[],u32 T1[],u32 T2[],u32 T3[]) {
	for (int i = 0; i < 256; i++) {
		T0[i] = Mul2(s[i]) << 24 | s[i] << 16 | s[i] << 8 | Mul3(s[i]);
		T1[i] = Mul3(s[i]) << 24 | Mul2(s[i]) << 16 | s[i] << 8 | s[i];
		T2[i] = s[i] << 24 | Mul3(s[i]) << 16 | Mul2(s[i]) << 8 | s[i];
		T3[i] = Mul3(s[i]) << 24 | s[i] << 16 | s[i] << 8 | s[i];
	}
}

void inv_TableGen() {
	for(int i=0;i<256;i++){
		invT0[i] = Mule(inv_s[i]) << 24 | Mul9(inv_s[i]) << 16 | Muld(inv_s[i]) << 8 | Mulb(inv_s[i]);
		invT1[i] = Mulb(inv_s[i]) << 24 | Mule(inv_s[i]) << 16 | Mul9(inv_s[i]) << 8 | Muld(inv_s[i]);
		invT2[i] = Muld(inv_s[i]) << 24 | Mulb(inv_s[i]) << 16 | Mule(inv_s[i]) << 8 | Mul9(inv_s[i]);
		invT3[i] = Mul9(inv_s[i]) << 24 | Muld(inv_s[i]) << 16 | Mulb(inv_s[i]) << 8 | Mule(inv_s[i]);
	};
}

void AES_Encryption();
void AES_Decryption();

#if KEY_LENGTH == 128
inline void AES_Encryption() {
	//Pre-Round
	u32 in0, in1, in2, in3;
	u32 out0, out1, out2, out3;
	u32 *w = W;

	in0 = P[0] ^ w[0];
	in1 = P[1] ^ w[1];
	in2 = P[2] ^ w[2];
	in3 = P[3] ^ w[3];

	//1Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[4];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[5];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[6];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[7];
	//2Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[8];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[9];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[10];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[11];
	//3Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[12];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[13];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[14];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[15];
	//4Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[16];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[17];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[18];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[19];
	//5Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[20];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[21];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[22];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[23];
	//6Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[24];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[25];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[26];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[27];
	//7Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[28];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[29];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[30];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[31];
	//8Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[32];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[33];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[34];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[35];
	//9Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[36];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[37];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[38];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[39];
	//Final Round
	P[0] = (s[out0 >> 24] << 24 | s[(out1 >> 16) & 0xff] << 16 | s[(out2 >> 8) & 0xff] << 8 | s[(out3 & 0xff)]) ^ w[40];
	P[1] = (s[out1 >> 24] << 24 | s[(out2 >> 16) & 0xff] << 16 | s[(out3 >> 8) & 0xff] << 8 | s[(out0 & 0xff)]) ^ w[41];
	P[2] = (s[out2 >> 24] << 24 | s[(out3 >> 16) & 0xff] << 16 | s[(out0 >> 8) & 0xff] << 8 | s[(out1 & 0xff)]) ^ w[42];
	P[3] = (s[out3 >> 24] << 24 | s[(out0 >> 16) & 0xff] << 16 | s[(out1 >> 8) & 0xff] << 8 | s[(out2 & 0xff)]) ^ w[43];
}
inline void AES_Decryption() {
	u32 in0, in1, in2, in3;
	u32 out0, out1, out2, out3;
	u32 *w = dW;
	//Pre-Round
	in0 = P[0] ^ w[0];
	in1 = P[1] ^ w[1];
	in2 = P[2] ^ w[2];
	in3 = P[3] ^ w[3];

	//1Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[4];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[5];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[6];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[7];
	//2Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[8];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[9];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[10];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[11];
	//3Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[12];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[13];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[14];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[15];
	//4Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[16];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[17];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[18];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[19];
	//5Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[20];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[21];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[22];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[23];
	//6Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[24];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[25];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[26];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[27];
	//7Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[28];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[29];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[30];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[31];
	//8Round

	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[32];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[33];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[34];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[35];
	//9Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[36];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[37];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[38];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[39];
	//Final Round
	P[0] = (inv_s[out0 >> 24] << 24 | inv_s[(out3 >> 16) & 0xff] << 16 | inv_s[(out2 >> 8) & 0xff] << 8 | inv_s[out1 & 0xff]) ^ w[40];
	P[1] = (inv_s[out1 >> 24] << 24 | inv_s[(out0 >> 16) & 0xff] << 16 | inv_s[(out3 >> 8) & 0xff] << 8 | inv_s[out2 & 0xff]) ^ w[41];
	P[2] = (inv_s[out2 >> 24] << 24 | inv_s[(out1 >> 16) & 0xff] << 16 | inv_s[(out0 >> 8) & 0xff] << 8 | inv_s[out3 & 0xff]) ^ w[42];
	P[3] = (inv_s[out3 >> 24] << 24 | inv_s[(out2 >> 16) & 0xff] << 16 | inv_s[(out1 >> 8) & 0xff] << 8 | inv_s[out0 & 0xff]) ^ w[43];
}
#elif KEY_LENGTH==192
inline void AES_Encryption() {
	//Pre-Round
	u32 in0, in1, in2, in3;
	u32 out0, out1, out2, out3;
	u32 *w = W;

	in0 = P[0] ^ w[0];
	in1 = P[1] ^ w[1];
	in2 = P[2] ^ w[2];
	in3 = P[3] ^ w[3];

	//1Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[4];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[5];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[6];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[7];
	//2Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[8];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[9];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[10];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[11];
	//3Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[12];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[13];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[14];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[15];
	//4Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[16];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[17];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[18];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[19];
	//5Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[20];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[21];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[22];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[23];
	//6Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[24];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[25];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[26];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[27];
	//7Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[28];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[29];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[30];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[31];
	//8Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[32];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[33];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[34];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[35];
	//9Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[36];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[37];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[38];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[39];
	//10Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[40];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[41];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[42];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[43];
	//11Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[44];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[45];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[46];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[47];
	//Final Round
	P[0] = (s[out0 >> 24] << 24 | s[(out1 >> 16) & 0xff] << 16 | s[(out2 >> 8) & 0xff] << 8 | s[(out3 & 0xff)]) ^ w[48];
	P[1] = (s[out1 >> 24] << 24 | s[(out2 >> 16) & 0xff] << 16 | s[(out3 >> 8) & 0xff] << 8 | s[(out0 & 0xff)]) ^ w[49];
	P[2] = (s[out2 >> 24] << 24 | s[(out3 >> 16) & 0xff] << 16 | s[(out0 >> 8) & 0xff] << 8 | s[(out1 & 0xff)]) ^ w[50];
	P[3] = (s[out3 >> 24] << 24 | s[(out0 >> 16) & 0xff] << 16 | s[(out1 >> 8) & 0xff] << 8 | s[(out2 & 0xff)]) ^ w[51];
}

inline void AES_Decryption() {
	u32 in0, in1, in2, in3;
	u32 out0, out1, out2, out3;
	u32 *w = dW;
	//Pre-Round
	in0 = P[0] ^ w[0];
	in1 = P[1] ^ w[1];
	in2 = P[2] ^ w[2];
	in3 = P[3] ^ w[3];

	//1Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[4];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[5];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[6];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[7];

	//2Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[8];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[9];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[10];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[11];
	//3Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[12];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[13];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[14];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[15];
	//4Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[16];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[17];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[18];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[19];
	//5Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[20];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[21];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[22];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[23];
	//6Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[24];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[25];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[26];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[27];
	//7Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[28];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[29];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[30];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[31];
	//8Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[32];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[33];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[34];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[35];
	//9Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[36];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[37];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[38];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[39];
	//10Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[40];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[41];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[42];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[43];
	//11Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[44];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[45];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[46];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[47];
	//Final Round
	P[0] = (inv_s[out0 >> 24] << 24 | inv_s[(out3 >> 16) & 0xff] << 16 | inv_s[(out2 >> 8) & 0xff] << 8 | inv_s[out1 & 0xff]) ^ w[48];
	P[1] = (inv_s[out1 >> 24] << 24 | inv_s[(out0 >> 16) & 0xff] << 16 | inv_s[(out3 >> 8) & 0xff] << 8 | inv_s[out2 & 0xff]) ^ w[49];
	P[2] = (inv_s[out2 >> 24] << 24 | inv_s[(out1 >> 16) & 0xff] << 16 | inv_s[(out0 >> 8) & 0xff] << 8 | inv_s[out3 & 0xff]) ^ w[50];
	P[3] = (inv_s[out3 >> 24] << 24 | inv_s[(out2 >> 16) & 0xff] << 16 | inv_s[(out1 >> 8) & 0xff] << 8 | inv_s[out0 & 0xff]) ^ w[51];
}
#elif KEY_LENGTH==256
inline void AES_Encryption() {
	//Pre-Round
	u32 in0, in1, in2, in3;
	u32 out0, out1, out2, out3;
	u32 *w = W;

	in0 = P[0] ^ w[0];
	in1 = P[1] ^ w[1];
	in2 = P[2] ^ w[2];
	in3 = P[3] ^ w[3];

	//1Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[4];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[5];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[6];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[7];
	//2Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[8];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[9];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[10];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[11];
	//3Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[12];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[13];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[14];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[15];
	//4Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[16];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[17];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[18];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[19];
	//5Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[20];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[21];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[22];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[23];
	//6Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[24];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[25];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[26];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[27];
	//7Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[28];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[29];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[30];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[31];
	//8Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[32];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[33];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[34];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[35];
	//9Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[36];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[37];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[38];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[39];
	//10Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[40];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[41];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[42];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[43];
	//11Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[44];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[45];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[46];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[47];
	//12Round
	in0 = Te0[out0 >> 24] ^ Te1[(out1 >> 16) & 0xff] ^ Te2[(out2 >> 8) & 0xff] ^ Te3[out3 & 0xff] ^ w[48];
	in1 = Te0[out1 >> 24] ^ Te1[(out2 >> 16) & 0xff] ^ Te2[(out3 >> 8) & 0xff] ^ Te3[out0 & 0xff] ^ w[49];
	in2 = Te0[out2 >> 24] ^ Te1[(out3 >> 16) & 0xff] ^ Te2[(out0 >> 8) & 0xff] ^ Te3[out1 & 0xff] ^ w[50];
	in3 = Te0[out3 >> 24] ^ Te1[(out0 >> 16) & 0xff] ^ Te2[(out1 >> 8) & 0xff] ^ Te3[out2 & 0xff] ^ w[51];
	//13Round
	out0 = Te0[in0 >> 24] ^ Te1[(in1 >> 16) & 0xff] ^ Te2[(in2 >> 8) & 0xff] ^ Te3[in3 & 0xff] ^ w[52];
	out1 = Te0[in1 >> 24] ^ Te1[(in2 >> 16) & 0xff] ^ Te2[(in3 >> 8) & 0xff] ^ Te3[in0 & 0xff] ^ w[53];
	out2 = Te0[in2 >> 24] ^ Te1[(in3 >> 16) & 0xff] ^ Te2[(in0 >> 8) & 0xff] ^ Te3[in1 & 0xff] ^ w[54];
	out3 = Te0[in3 >> 24] ^ Te1[(in0 >> 16) & 0xff] ^ Te2[(in1 >> 8) & 0xff] ^ Te3[in2 & 0xff] ^ w[55];
	//Final Round
	P[0] = (s[out0 >> 24] << 24 | s[(out1 >> 16) & 0xff] << 16 | s[(out2 >> 8) & 0xff] << 8 | s[(out3 & 0xff)]) ^ w[56];
	P[1] = (s[out1 >> 24] << 24 | s[(out2 >> 16) & 0xff] << 16 | s[(out3 >> 8) & 0xff] << 8 | s[(out0 & 0xff)]) ^ w[57];
	P[2] = (s[out2 >> 24] << 24 | s[(out3 >> 16) & 0xff] << 16 | s[(out0 >> 8) & 0xff] << 8 | s[(out1 & 0xff)]) ^ w[58];
	P[3] = (s[out3 >> 24] << 24 | s[(out0 >> 16) & 0xff] << 16 | s[(out1 >> 8) & 0xff] << 8 | s[(out2 & 0xff)]) ^ w[59];
}

inline void AES_Decryption() {
	u32 in0, in1, in2, in3;
	u32 out0, out1, out2, out3;
	u32 *w = dW;
	//Pre-Round
	in0 = P[0] ^ w[0];
	in1 = P[1] ^ w[1];
	in2 = P[2] ^ w[2];
	in3 = P[3] ^ w[3];

	//1Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[4];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[5];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[6];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[7];

	//2Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[8];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[9];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[10];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[11];
	//3Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[12];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[13];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[14];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[15];
	//4Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[16];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[17];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[18];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[19];
	//5Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[20];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[21];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[22];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[23];
	//6Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[24];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[25];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[26];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[27];
	//7Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[28];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[29];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[30];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[31];
	//8Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[32];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[33];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[34];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[35];
	//9Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[36];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[37];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[38];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[39];
	//10Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[40];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[41];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[42];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[43];
	//11Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[44];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[45];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[46];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[47];
	//12Round
	in0 = invT0[out0 >> 24] ^ invT3[(out1 & 0xff)] ^ invT2[(out2 >> 8) & 0xff] ^ invT1[(out3 >> 16) & 0xff] ^ w[48];
	in1 = invT0[out1 >> 24] ^ invT3[(out2 & 0xff)] ^ invT2[(out3 >> 8) & 0xff] ^ invT1[(out0 >> 16) & 0xff] ^ w[49];
	in2 = invT0[out2 >> 24] ^ invT3[(out3 & 0xff)] ^ invT2[(out0 >> 8) & 0xff] ^ invT1[(out1 >> 16) & 0xff] ^ w[50];
	in3 = invT0[out3 >> 24] ^ invT3[(out0 & 0xff)] ^ invT2[(out1 >> 8) & 0xff] ^ invT1[(out2 >> 16) & 0xff] ^ w[51];
	//13Round
	out0 = invT0[in0 >> 24] ^ invT3[(in1 & 0xff)] ^ invT2[(in2 >> 8) & 0xff] ^ invT1[(in3 >> 16) & 0xff] ^ w[52];
	out1 = invT0[in1 >> 24] ^ invT3[(in2 & 0xff)] ^ invT2[(in3 >> 8) & 0xff] ^ invT1[(in0 >> 16) & 0xff] ^ w[53];
	out2 = invT0[in2 >> 24] ^ invT3[(in3 & 0xff)] ^ invT2[(in0 >> 8) & 0xff] ^ invT1[(in1 >> 16) & 0xff] ^ w[54];
	out3 = invT0[in3 >> 24] ^ invT3[(in0 & 0xff)] ^ invT2[(in1 >> 8) & 0xff] ^ invT1[(in2 >> 16) & 0xff] ^ w[55];
	//Final Round
	P[0] = (inv_s[out0 >> 24] << 24 | inv_s[(out3 >> 16) & 0xff] << 16 | inv_s[(out2 >> 8) & 0xff] << 8 | inv_s[out1 & 0xff]) ^ w[56];
	P[1] = (inv_s[out1 >> 24] << 24 | inv_s[(out0 >> 16) & 0xff] << 16 | inv_s[(out3 >> 8) & 0xff] << 8 | inv_s[out2 & 0xff]) ^ w[57];
	P[2] = (inv_s[out2 >> 24] << 24 | inv_s[(out1 >> 16) & 0xff] << 16 | inv_s[(out0 >> 8) & 0xff] << 8 | inv_s[out3 & 0xff]) ^ w[58];
	P[3] = (inv_s[out3 >> 24] << 24 | inv_s[(out2 >> 16) & 0xff] << 16 | inv_s[(out1 >> 8) & 0xff] << 8 | inv_s[out0 & 0xff]) ^ w[59];
}
#endif




void KeyExpansion(u8 * key, int N) {

	u32 temp;

	for (int i = 0; i < N; i++)
		W[i] = (key[4 * i] << 24 | key[4 * i + 1] << 16 | key[4 * i + 2] << 8 | key[4 * i + 3]);

	for (int i = N; i < 4 * (N + 7); i++) {
		temp = W[i - 1];
		if (i % N == 0) {
			temp = SubWord(RotWord(temp));
			temp = temp ^ RCon[i / N - 1];
		}
		else if (N > 6 && i%N == 4)
			temp = SubWord(temp);
		W[i] = W[i - N] ^ temp;
	}

	for (int i = 0; i < 4 * (N + 7); i += 4) {
		dW[i] = W[4*(N+6) - i];
		dW[i + 1] = W[4*(N+6) - i+1];
		dW[i + 2] = W[4*(N+6) - i+2];
		dW[i + 3] = W[4*(N+6) - i+3];
	}

	for (int i = 4; i < 4 * (N + 6); i++) {
		u32 tmp[4], tmp2[4];
		tmp[0] = dW[i] >> 24;
		tmp[1] = (dW[i] >> 16) & 0xff;
		tmp[2] = (dW[i] >> 8) & 0xff;
		tmp[3] = dW[i] & 0xff;

		tmp2[0] = Mule(tmp[0]) ^ Mulb(tmp[1]) ^ Muld(tmp[2]) ^ Mul9(tmp[3]);
		tmp2[1] = Mul9(tmp[0]) ^ Mule(tmp[1]) ^ Mulb(tmp[2]) ^ Muld(tmp[3]);
		tmp2[2] = Muld(tmp[0]) ^ Mul9(tmp[1]) ^ Mule(tmp[2]) ^ Mulb(tmp[3]);
		tmp2[3] = Mulb(tmp[0]) ^ Muld(tmp[1]) ^ Mul9(tmp[2]) ^ Mule(tmp[3]);

		dW[i] = tmp2[0] << 24 | tmp2[1] << 16 | tmp2[2] << 8 | tmp2[3];
	}

	
}

int main(void) {

	u32 T0[256],T1[256],T2[256],T3[256];
	TableGen(T0,T1,T2,T3);
	inv_TableGen(invT0, invT1, invT2, invT3);
	KeyExpansion(key, KEY_LENGTH>>5);

	/*--------------------------------------------------- Manual----------------------------------------------------------
	l																													  l
	l							You can select 128,192,256 bits AES by substituting "#define KEY_LENGTH 128" at line 8	  l
	l                           Plaintext(128bits) :  P[0], P[1], P[2], P[3]											  l
	l                           Encryption         :  AES_Encryption()													  l
	l                           Decryption         :  AES_Decrpytion()													  l
	l                           Ciphertext(128bits):  P[0], P[1], P[2], P[3]											  l
	l																													  l
	l---------------------------------------------------------------------------------------------------------------------*/
	
	u32 i, j, iter = 5000000;
	clock_t start, finish;
	double time;

	// before test
	printf("AES Speed TEST\n");
	printf("%08x %08x %08x %08x\n", P[0], P[1], P[2], P[3]);

	for (i = 0; i < 1; i++) {
		printf("- TEST (%d) : ", i + 1);
		start = clock();
		AES_Encryption();

		finish = clock();
		time = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("%.3f Gbps\n", (double)(128 * 2 * iter) / (time * 1024 * 1024 * 1024));
	}
	printf("%08X %08X %08X %08X\n", P[0], P[1], P[2], P[3]);

	/* ------------------------------------------------- speed test -----------------------------------------------
										It's 20% faster than traditional AES algorithm. 

	for (i = 0; i < 5; i++) {
		printf("- TEST (%d) : ", i + 1);
		start = clock();
		for (j = iter; j; j--) {
			AES_Encryption();
			AES_Encryption();
		}
		finish = clock();
		time = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("%.3f Gbps\n", (double)(128 * 2 * iter) / (time * 1024 * 1024 * 1024));
	}
	printf("%08X %08X %08X %08X\n", P[0], P[1], P[2], P[3]);
	/ -------------------------------------------------------------------------------------------------------------*/
	return 0;
}

