#include "StdAfx.h"
#include "VhclConverter.h"

#include "SimuImage.h"

void VhclConverter::convertImage(PakNode *node, int index, int targetTileSize, int waytype) const
{
	SimuImage image;
	image.load(*node->data());
	image.x -= OFFSET_X;
	image.y -= OFFSET_Y;
	image.save(*node->data());
}

void VhclConverter::convertImageList(PakNode *node, int targetTileSize, int waytype) const
{
	int i = 0;
	for(PakNode::iterator it = node->begin(); it != node->end(); it++, i++)
		convertImage(*it, i, targetTileSize, waytype);
}

void VhclConverter::convertImageList2(PakNode *node, int targetTileSize, int waytype) const
{
	for(PakNode::iterator it = node->begin(); it != node->end(); it++)
	{
		if((*it)->type() == "IMG1")
			convertImageList(*it, targetTileSize, waytype);
	}
}

const unsigned short DEFAULT_INTRO_DATE = 1900;
const unsigned short DEFAULT_RETIRE_DATE= 2999;

const unsigned char vehikel_besch_t_steam = 0;
const unsigned char vehikel_besch_t_diesel = 1;
const unsigned char vehikel_besch_t_electric = 2;

#pragma pack(push, 1)
struct _PakVhclFieldsVer7
{
	unsigned short version;
	unsigned long cost;
	unsigned short payload;
	unsigned short speed;
	unsigned short weight;
	unsigned long power;
	unsigned short runningCost;
	unsigned short introMonth;
	unsigned short retireMonth;
	unsigned short gear;
	unsigned char waytype;
	unsigned char sound;
	unsigned char enginetype;
	unsigned char length;
	unsigned char prevs;
	unsigned char nexts;
};
struct _PakVhclFieldsVer9
{
	unsigned short version;
	unsigned long cost;
	unsigned short payload;
	unsigned short loading_time; // new
	unsigned short speed;
	unsigned short weight;
	unsigned short axle_load; // new
	unsigned long power;
	unsigned short runningCost;
	unsigned short fixed_cost; // new
	unsigned short introMonth;
	unsigned short retireMonth;
	unsigned short gear;
	unsigned char waytype;
	unsigned char sound;
	unsigned char enginetype;
	unsigned char length;
	unsigned char prevs;
	unsigned char nexts;
	unsigned char freight_image_type; 
};
struct _PakVhclFieldsVer10
{
	unsigned short version;
	unsigned long cost;
	unsigned short payload;
	unsigned short loading_time;
	unsigned short speed;
	unsigned long weight;
	unsigned short axle_load;
	unsigned long power;
	unsigned short runningCost;
	unsigned short fixed_cost;
	unsigned short introMonth;
	unsigned short retireMonth;
	unsigned short gear;
	unsigned char waytype;
	unsigned char sound;
	unsigned char enginetype;
	unsigned char length;
	unsigned char prevs;
	unsigned char nexts;
	unsigned char freight_image_type; 
};
#pragma pack(pop)

enum waytype_t {
	invalid_wt       =  -1,
	ignore_wt        =   0,
	road_wt          =   1,
	track_wt         =   2,
	water_wt         =   3,
	overheadlines_wt =   4,
	monorail_wt      =   5,
	maglev_wt        =   6,
	tram_wt          =   7,
	narrowgauge_wt   =   8,
	air_wt           =  16,
	powerline_wt     = 128
};

/** ver3�ȑO��waytype��V�����`���ɕϊ����� */
unsigned char waytype_old_to_new(int value, bool &electric)
{
	static const waytype_t convert_from_old[8] = {road_wt, track_wt, water_wt, air_wt, invalid_wt, monorail_wt, invalid_wt, tram_wt };
	if(value == 4)
	{
		electric = true;
		value = 1;
	}
	return static_cast<unsigned char>(convert_from_old[value]);
}

inline unsigned char readUI8(char *&p)
{
	return *(p++);
}
inline unsigned short readUI16(char*&p)
{
	unsigned short val = *pointer_cast<unsigned short*>(p);
	p += 2;
	return val;
}

inline unsigned long readUI32(char*&p)
{
	unsigned long val = *pointer_cast<unsigned long*>(p);
	p += 4;
	return val;
}

int loadOldVhclHeader(_PakVhclFieldsVer7 &fields, int version, char* p)
{
	char *top = p;
	unsigned short tmp;
	bool electricEngine = false;

	fields.version = (version > 0)? readUI16(p): 0;
	int prev_waytype = (version > 0)? 0: readUI16(p);
	if(version == 0){
		fields.payload = readUI16(p);
		fields.cost    = readUI32(p);
	}else{
		fields.cost    = readUI32(p);
		fields.payload = readUI16(p);
	}
	fields.speed       =                readUI16(p);                                // ���x
	fields.weight      =                readUI16(p);                                // �d��
	fields.power       =(version >= 6)? readUI32(p):                                // �o�́Bver6����32bit�Ɋg��
	                                    readUI16(p);
	fields.runningCost =                readUI16(p);                                // �ێ���
	fields.introMonth  =(version >= 5)? readUI16(p):                                // �o��N���Bver1�ȍ~�o��Bver5�ȍ~�̓f�[�^�`���ύX
	                    (version >= 1)? (tmp=readUI16(p),(tmp/16)*12 + (tmp%16)):
	                                    DEFAULT_INTRO_DATE*12;
	fields.retireMonth =(version >= 5)? readUI16(p):                                // ���ޔN���Bver3�ȍ~�o��Bver5�ȍ~�̓f�[�^�`���ύX
	                    (version >= 3)? (tmp=readUI16(p),(tmp/16)*12 + (tmp%16)):
	                                    DEFAULT_RETIRE_DATE*12;
	fields.gear       = (version >= 6)? readUI16(p):                                // �M�A��Bver6����32bit�Ɋg��
	                    (version >= 1)? readUI8(p):
	                                    64;
	fields.waytype    = (version >= 4)? readUI8(p):                                     // ��蕨�̃^�C�v
	                    (version >= 1)? waytype_old_to_new(readUI8(p), electricEngine): // ver4���̂�waytype�̌`�����Ⴄ
	                                    waytype_old_to_new(prev_waytype,electricEngine);// ver1�͋L�^�ꏊ���Ⴄ
	fields.sound      = (version >= 1)? readUI8(p):
	                                    static_cast<unsigned char>(readUI16(p));
	unsigned char
	   prev_enginetype= (version >= 6)? readUI8(p):                                    //
	                                    0;
	fields.length     = (version >= 7)? readUI8(p):                                    // �ԗ��̒����Bver7�ȍ~�o��B
	                                    8;
	fields.prevs      = (version >= 1)? readUI8(p):                                    // �O���A���\�ԗ��Bver1�̂�16bit
	                                    static_cast<unsigned char>(readUI16(p));
	fields.nexts      = (version >= 1)? readUI8(p):                                    // ����A���\�ԗ��Bver1�̂�16bit
	                                    static_cast<unsigned char>(readUI16(p));
	fields.enginetype = (version >= 6)? prev_enginetype:                               
	                    (version >= 2)? readUI8(p):                                    // ver1�ɂ̓G���W���`�����Ȃ��̂ŉ����琄������
	                                    ((fields.sound==3) ? vehikel_besch_t_steam : vehikel_besch_t_diesel);

	if(electricEngine) fields.enginetype = vehikel_besch_t_electric;

	return p - top;
}

void VhclConverter::convertVhclAddon(PakNode *node,  int targetTileSize) const
{
	unsigned char waytype;

	char* data = pointer_cast<char*>(node->dataP());
	int version = getPakNodeVer(*pointer_cast<unsigned short*>(data));
	switch(version)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		{
			//ver0 �` ver6�̃w�b�_�ɂ�length�����݂��Ȃ��̂�ver7�w�b�_�ɕϊ��������length��{�ɂ���B
			_PakVhclFieldsVer7 ver7h;
			int headerSize = loadOldVhclHeader(ver7h, version, data);
			ver7h.version = 0x8007;
			ver7h.length  = ver7h.length*2;
			node->data()->insert(node->data()->begin(), sizeof(_PakVhclFieldsVer7) - headerSize, 0);
			*pointer_cast<_PakVhclFieldsVer7*>(node->dataP()) = ver7h;

			waytype = ver7h.waytype;
			break;
		}
	case 7:
	case 8:
		{
			// ver7����Alength���ł����̂�2�{�ɂ���
			// ver8�́Afreight_image_type�����ɒǉ����ꂽ�����Ȃ̂ŁA���l�̃R�[�h��OK
			_PakVhclFieldsVer7 *ver7h = pointer_cast<_PakVhclFieldsVer7*>(data);
			ver7h->length = ver7h->length * 2;

			waytype = ver7h->waytype;
			break;
		}
	  case 9:
		{
			// new: fixed_cost, loading_time, axle_load
			_PakVhclFieldsVer9 *ver9h = pointer_cast<_PakVhclFieldsVer9*>(data);
			ver9h->length = ver9h->length * 2;

			waytype = ver9h->waytype;
			break;
		}
	  case 10:
		{
			// new: weight in kgs
			_PakVhclFieldsVer10 *ver10h = pointer_cast<_PakVhclFieldsVer10*>(data);
			ver10h->length = ver10h->length * 2;

			waytype = ver10h->waytype;
			break;
		}
		
	default:
		throw std::runtime_error("resizeobj���Ή����Ă��Ȃ�makeobj�ō쐬���ꂽPAK�t�@�C���ł�");
	}

	for(PakNode::iterator it = node->begin(); it != node->end(); it++)
	{
		if((*it)->type() == "IMG1")
			convertImageList(*it, targetTileSize, waytype);
		else if ((*it)->type() == "IMG2")
			convertImageList2(*it, targetTileSize, waytype);
	}
}
