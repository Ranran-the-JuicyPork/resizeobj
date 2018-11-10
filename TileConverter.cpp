#include "stdafx.h"
#include "TileConverter.h"
#include "VhclConverter.h"

const char *TILECONV_ERROR_UNSUPPORTED_VERSION = "�������[�h���Ή����Ă��Ȃ��o�[�W������PAK�t�@�C���ł�";
const char *TILECONV_ERROR_UNSUPPORTED_TYPE    = "�������[�h���Ή����Ă��Ȃ��`����PAK�t�@�C���ł�";

// haus_besch.h(rev.2615)����
enum utyp
{
	unbekannt         =  0, // �s��
	attraction_city   =  1, // �s�����ꌚ�z��
	attraction_land   =  2, // �x�O���ꌚ�z��
	denkmal           =  3, // �L�O��
	fabrik            =  4, // �H��
	rathaus           =  5, // ����
	weitere           =  6, // name�ŋ@�\���w�肵������̉w�{�݁H
	firmensitz        =  7, // �{��
	bahnhof           =  8, // �w
	bushalt           =  9, // �o�X��
	ladebucht         = 10, // ���[�h
	hafen             = 11, // �`
	binnenhafen       = 12, // �^�͍`
	airport           = 13, // ��`
	monorailstop      = 14, // ���m���[���w
	bahnhof_geb       = 16, // �w�g��
	bushalt_geb       = 17, // �o�X��g��
	ladebucht_geb     = 18, // ���[�h�g��
	hafen_geb         = 19, // �`�g��
	binnenhafen_geb   = 20, // �^�͍`�g��
	airport_geb       = 21, // ��`�g��
	monorail_geb      = 22, // ���m���[���w�g��
	wartehalle        = 30, // �ҍ���
	post              = 31, // �X�֋�
	lagerhalle        = 32, // �q��
	depot             = 33, // �Ԍ�
	generic_stop      = 34, // �ėp���
	generic_extension = 35, // �ėp�g��
	last_haus_typ,
	unbekannt_flag    = 128, // �s���t���O�H
};

const char *utypNames[] = {
	"res,com,ind",
	"cur(city)",
	"cur(land)",
	"mon",
	"factory",
	"tow",
	"__weitere",
	"hq",
	"station",
	"busstop",
	"carstop",
	"habour",
	"wharf",
	"airport",
	"manorailstop",
	"",
	"station(extension)",
	"busstop(extension)",
	"carstop(extension)",
	"habour(extension)",
	"wharf(extension)",
	"airport(extension)",
	"manorailstop(extension)",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"hall",
	"post",
	"shed",
	"depot",
	"stop",
	"extension"};

inline const char *utype2str(utyp u)
{
	if(u<lengthof(utypNames))
		return utypNames[u];
	else
		return "";
}

void TileConverter::convertNodeTree(PakNode *node) const
{
	if(node->type()=="BUIL")
	{
		convertBuil(node);
	}
	else if(node->type() == "FIEL")
	{
		m_ic->convertAddon(node);
	}
	else if(node->type() == "SMOK")
	{
		convertSmokeTreeImage(node);
	}
	else if(node->type() == "FSMO")
	{
		convertFactorySmoke(node);
	}
	else
	{
		for(PakNode::iterator it = node->begin(); it != node->end(); it++) convertNodeTree(*it);
	}
}

void TileConverter::convertFactorySmoke(PakNode *node) const
{
	PakFactorySmoke* pfs = &node->data_p()->fsmo;
	// SmokeTile�͒P���ɔ{�ɂ���
	pfs->tileX *= 2;
	pfs->tileY *= 2;
	// SmokeOffset�̒P�ʂ�(�^�C���T�C�Y/64)�Ȃ悤�Ȃ̂ŁA������{�ɂ���B
	pfs->offsetX *= 2;
	pfs->offsetY *= 2;
	
	if(pfs->offsetX-3 < -128 || 127 < pfs->offsetX+3
		|| pfs->offsetY-13 < -128 || 127 < pfs->offsetY+3)
	{
		// SmokeOffset.x +(-3�`+3) �� SmokeOffset.y +(-13�`3)��sint8�͈̔͂łȂ��ƍ���
		const char *emptyXREF = "SMOK\0\0";
		node->clear();
		node->type("XREF");
		node->data()->assign(emptyXREF, emptyXREF + 6);
	}
}

void TileConverter::convertSmokeTreeImage(PakNode *node) const
{
	if(node->type()=="IMG")
	{
		SimuImage image;
		image.load(*node->data());

		image.x -= m_ic->oldTileSize()/4;
		image.y -= m_ic->oldTileSize()/4;
		image.save(*node->data());
	}
	else
	{
		for(PakNode::iterator it = node->begin(); it != node->end(); it++) convertSmokeTreeImage(*it);
	}
}


void TileConverter::convertBuil(PakNode *node) const
{
	/*
	BUIL
	�� TEXT[���2?]
	�� TILE[���C�A�E�g�~�����~�c��]
	�@ �� IMG2[�G�߁~�O��]
	�@�@�@ �� IMG1[����]
	�@�@�@�@�@ �� IMG[�A�j���[�V����]
	*/
	PakBuilding *header = &node->data_p()->buil;

	int layouts, x, y;
	utyp ut;

	// �w�b�_����K�v�Ȓl������Ă���ƂƂ���Dims��ϊ�
	switch(GetPakNodeVer(header->ver1_5.version))
	{
	case 0:
		ut = static_cast<utyp>(header->ver0.utype);
		x = header->ver0.x;
		y = header->ver0.y;
		layouts = header->ver0.layouts;
		header->ver0.x = x*2;
		header->ver0.y = y*2;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		ut = static_cast<utyp>(header->ver1_5.utype);
		x = header->ver1_5.x;
		y = header->ver1_5.y;
		layouts = header->ver1_5.layouts;
		header->ver1_5.x = x*2;
		header->ver1_5.y = y*2;
		break;
	default:
		throw std::runtime_error(TILECONV_ERROR_UNSUPPORTED_VERSION);
	}
	// �����^�C���ɑΉ����Ă��錚���`�����`�F�b�N����
	switch(ut)
	{
	case attraction_city:
	case attraction_land:
	case fabrik:
	case rathaus:
	case firmensitz:
	case bahnhof_geb:
	case bushalt_geb:
	case ladebucht_geb:
	case hafen_geb:
	case binnenhafen_geb:
	case airport_geb:
	case monorail_geb:
	case wartehalle:
	case post:
	case lagerhalle:
	case generic_extension:
		// �������[
		break;
	default:
		std::ostringstream os;
		os << (ut<last_haus_typ ? TILECONV_ERROR_UNSUPPORTED_TYPE : TILECONV_ERROR_UNSUPPORTED_VERSION);
		os << "\nType:" << utype2str(ut) << " (" << static_cast<int>(ut) << ")"; 
		throw std::runtime_error(os.str());
	}

	// TILE�m�[�h�̑O�ɂ���TEXT/XREF�m�[�h�̐��𐔂��Ă���
	int t = 0;
	for(PakNode::iterator it = node->begin(); it != node->end(); it++)
	{
		std::string nodeType = (*it)->type();
		if(nodeType != "TEXT" && nodeType != "XREF") break;
		t++;
	}

	std::vector<PakNode*> *nodes = node->items();
	// �q�m�[�h���g��
	nodes->reserve(nodes->size()*4);
	for(int il=0; il<layouts; il++)
	{
		int height = il&1 ? x : y;
		int width  = il&1 ? y : x;

		for(int iy=0; iy<height; iy++)
		{
			PakNode::iterator it = nodes->begin() + t + (iy + il*height)*width*4;
			for(int ix=0; ix<width; ix++) it = nodes->insert(it+1, static_cast<PakNode*>(0)) + 1;
			nodes->insert(it, width*2, 0);
		}
	}

	// TILE�m�[�h��ϊ�
	PakNode::iterator it  = nodes->begin()+t;
	for(int il=0; il<layouts; il++)
	{
		int h = il&1 ? x : y;
		int w  = il&1 ? y : x;

		for(int iy=0; iy<h; iy++)
		{
			for(int ix=0; ix<w; ix++)
			{
				// �Â��^�C���m�[�h�P����V�����^�C���m�[�h�S���쐬
				PakNode *oldTileNode = it[0];
				PakNode *newTileNodes[4];
				for(int i=0; i<4; i++) newTileNodes[i] = new PakNode("TILE");
				it[    0] = newTileNodes[0];
				it[    1] = newTileNodes[1];
				it[w*2  ] = newTileNodes[2];
				it[w*2+1] = newTileNodes[3];

				convertTile(il, ix, iy, newTileNodes, oldTileNode, w*2, h*2);

				delete oldTileNode;
				it += 2;
			}
			it += w*2;
		}
	}

	// �J�[�\���̏k���E�A�C�R���摜�̊��荞��
	assert(m_ic);
	for(PakNode::iterator it = node->begin(); it != node->end(); it++)
	{
		PakNode *node = *it;
		if(node->type()=="CURS") m_ic->convertAddon(node);
	}
};

void TileConverter::convertTile(int layout, int x, int y,
		PakNode *tiles[], PakNode *srcTile, int width, int height) const
{
	if(srcTile->type()!="TILE")
		throw std::logic_error(TILECONV_ERROR_UNSUPPORTED_TYPE);

	int seasons=1, phasen;

	// ���m�[�h����K�v�Ȓl���擾����B
	PakTile *header = &srcTile->data_p()->tile;
	switch(GetPakNodeVer(header->ver1_2.version))
	{
	case 2:
		seasons = header->ver2.seasons;
	case 1:
		if(m_noAnimation) header->ver1_2.phasen = 1;
		phasen  = header->ver1_2.phasen;
		break;
	case 0:
		if(m_noAnimation) header->ver0.phasen = 1;
		phasen = header->ver0.phasen;
		break;
	default:
		throw std::runtime_error(TILECONV_ERROR_UNSUPPORTED_VERSION);
	}

	for(int i = 0;i<4; i++)
	{
		// �V�����m�[�h��data�����̂܂܃R�s�[���āA
		*tiles[i]->data() = *srcTile->data();
		PakTile *header = &tiles[i]->data_p()->tile;

		// index�����ύX����B
		int newIndex = (x*2+(i&1)) + ((y*2+i/2)+layout*height)*width;
		switch(GetPakNodeVer(header->ver1_2.version))
		{
		case 0:
			header->ver0.index = newIndex;
			break;
		case 1:
		case 2:
			header->ver1_2.index = newIndex;
		}
	}

	// �V�[�Y�����~(Front/Back�p=2)����IMG2�����݂���
	PakNode::iterator it = srcTile->begin();
	for(int i = 0; i<seasons*2; i++)
	{
		PakNode *newImg2Nodes[4];
		for(int j=0; j<4; j++)
			tiles[j]->appendChild(newImg2Nodes[j] = new PakNode("IMG2"));

		// IMG2��ϊ�
		convertImg2(x, y, newImg2Nodes, *it, phasen);

		it++;
	}
}

bool hasOpaquePixel(const Bitmap<PIXVAL> &bmp, int x, int y, int width, int height)
{
	for(int i = y; i<y+height; i++)
	{
		Bitmap<PIXVAL>::const_iterator it = bmp.begin(i) + x;
		Bitmap<PIXVAL>::const_iterator endIt = it + width;

		if(endIt != std::find_if(it, endIt, std::bind1st(std::not_equal_to<PIXVAL>(), SIMU_TRANSPARENT)))
			return true;
	}
	return false;
}

int getTileHeight(const Bitmap<PIXVAL> &bmp, int x, int y, int tileWidth, int tileHeight, int maxHeight)
{
	for(int i = maxHeight; i>0; i--)
	{
		if(hasOpaquePixel(bmp, x, y + tileHeight*(maxHeight-i), tileWidth, tileHeight))
			return i;
	}
	return 1;
}

void TileConverter::convertImg2(int x, int y, PakNode *img2s[], PakNode *srcImg2, int phasen) const
{
	if(srcImg2->type()!="IMG2")
		throw std::logic_error(TILECONV_ERROR_UNSUPPORTED_TYPE);

	// �^�C���T�C�Y��version�����o���B
	int oldHeight = srcImg2->length();
	int version;
	bool hasImage = false;
	for(PakNode::iterator it = srcImg2->begin(); it != srcImg2->end(); it++)
	{
		PakNode *imgNode  = *(*it)->begin();
		SimuImage img;
		img.load(*imgNode->data());
		if(0<img.width)
		{
			version = img.version;
			hasImage = true;
			break;
		}
	}
	if(!hasImage)
	{
		// �摜���w�肳��Ă��Ȃ�������󔒉摜���w�肳��Ă����ꍇ�́A
		// �m�[�h���R�s�[���邾���ŏI���B
		for(int i=0; i<4; i++)
		{
			*img2s[i]->data() = *srcImg2->data();
			for(PakNode::iterator it = srcImg2->begin(); it != srcImg2->end(); it++)
				img2s[i]->appendChild((*it)->clone());
		}
		return;
	}

	std::vector<Bitmap<PIXVAL>*> bitmaps;
	// �摜�f�[�^���r�b�g�}�b�v�ɓW�J����B
	for(int i = 0; i<phasen; i++)
	{
		Bitmap<PIXVAL> *bmp = new MemoryBitmap<PIXVAL>(m_ic->oldTileSize()*2, m_ic->oldTileSize() * (oldHeight+1));
		bitmaps.push_back(bmp);
		bmp->clear(SIMU_TRANSPARENT);

		for(int j = 0; j<oldHeight; j++)
		{
			PakNode *img1Node = (*srcImg2)[j];
			PakNode *imgNode = (*img1Node)[std::min(i, (int)img1Node->length()-1)];
			SimuImage img;
			img.load(*imgNode->data());

			int imgx, imgy, imgw, imgh;
			img.getBounds(imgx, imgy, imgw, imgh);
			img.drawTo(imgx, imgy + (oldHeight-j)*m_ic->oldTileSize(), *bmp);
		}
	}

	/*

	�@��������������������
	�@���@�@�@�@�@�@�@�@��
	�@���@�@�@�@�@�@�@�@��
	�@���@�@�@�@�@�@�@�@��
	�@��������������������
	�@��������������������
	�@��������������������
	 �`�`�`�`�`�`�`�`�`�`�`
	 �`�`�`�`�`�`�`�`�`�`�`
	�@��������������������
	�@��������������������
	�@��������������������
	�@��������������������
	�@�b������������������  0���c[2n  ][2m  ]�c �� ��
	�@�b������������������  1���c[2n+1][2m  ]�c �� �����c
	�@�b������������������  2���c[2n  ][2m+1]�c �� �����c
	�@�b������������������  3���c[2n+1][2m+1]�c �� ��
	�@��������������������
	�@���@�@���������@�@��
	�@��������������������
	*/
	const int E=m_ic->oldTileSize()/8;
	const int by=oldHeight*m_ic->oldTileSize();

	encodeImg2(img2s[0], bitmaps, 2*E, by + 2*E,           1, 4*E, 3*E, version);
	encodeImg2(img2s[3], bitmaps, 2*E, by + 4*E,           1, 4*E, 4*E, version);
	encodeImg2(img2s[1], bitmaps, 4*E,      7*E, oldHeight*2, 4*E, 4*E, version);
	encodeImg2(img2s[2], bitmaps, 0  ,      7*E, oldHeight*2, 4*E, 4*E, version);

	for(std::vector<Bitmap<PIXVAL>*>::iterator it = bitmaps.begin(); it != bitmaps.end(); it++)
		delete *it;
};

void TileConverter::encodeImg2(PakNode *img2, std::vector<Bitmap<PIXVAL>*> &bitmaps,
		int bx, int by, int maxHeight, int width, int height, int version) const
{
	// �V���������̌���
	int newHeight = 1;
	for(unsigned int i = 0; i<bitmaps.size(); i++)
		newHeight = std::max(newHeight, getTileHeight(*bitmaps[i], bx, by, width, height, maxHeight));

	// ���肵���V���������Ɋ�Â��āAIMG2�m�[�h�w�b�_�̏��������B IMG1�m�[�h��ǉ�
	img2->data()->resize(4);
	unsigned short *img2Header = &img2->data_p()->img2;
	img2Header[0] = newHeight;
	img2Header[1] = 0;

	for(int ih = 0; ih<newHeight; ih++)
	{
		PakNode *img1Node = new PakNode("IMG1");
		img2->appendChild(img1Node);
		img1Node->data()->resize(4);
		unsigned short *img1Header = &img1Node->data_p()->img1;
		img1Header[0] = bitmaps.size();
		img1Header[1] = 0;

		for(unsigned int ip=0; ip<bitmaps.size(); ip++)
		{
			PakNode *imgNode = new PakNode("IMG");
			img1Node->appendChild(imgNode);
			
			Bitmap<PIXVAL> bmp(*bitmaps[ip], bx, by + (maxHeight-ih-1)*height, width, height);

			// �G���R�[�h
			SimuImage image;
			image.version = version;
			image.zoomable= true;
			image.encodeFrom(bmp, 0, 0, false);
			image.save(*imgNode->data());

			// �G���R�[�h���������̓N���A
			bmp.clear(SIMU_TRANSPARENT);
		}
	}
}

