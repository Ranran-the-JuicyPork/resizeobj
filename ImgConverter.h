#pragma once

#include "paknode.h"
#include "bitmap.h"
#include "SimuImage.h"

enum SCConvMode{
	scmNONE,
	scmTOPLEFT,
	scmTWO,
};

class ImgConverter
{
public:
	static const int MAX_ALPHA = 100;

	void convertAddon(PakNode *node) const{convertNodeTree(node); };

	int alpha() const{ return m_alpha; }
	void alpha(int value){ m_alpha = value; }
	SCConvMode specialColorMode() const{return m_specialColorMode; }
	void specialColorMode(SCConvMode value){ m_specialColorMode = value; }
private:
	int m_alpha;
	SCConvMode m_specialColorMode;

	/// IMG�m�[�h��T����convertImage���AFSMO�m�[�h��T����convertFactorySmoke���ĂԁB
	bool convertNodeTree(PakNode *node) const;
	/// �摜��ϊ�����Bzoomable�Ȃ�k���B�łȂ���Η]���؂�̂�
	void convertImage(PakNode *node) const;
	/// �摜���k�����ă^�C���T�C�Y�𔼌�����B
	void shrinkImage(SimuImage &data) const;
	/// �E�E����TileSize/2�ȏ�]��������ꍇ�͂��̗]����؂�̂Ăă^�C���T�C�Y�𔼌�����B
	/// �����ɃA�C�R���摜�p
	bool cutImageMargin(SimuImage &image) const;
	/// �Y�Ǝ{�݂̉��̈ʒu�𒲐�����c�c��������
	void convertFactorySmoke(PakNode *node) const;
	/// 4�s�N�Z������������1�s�N�Z�����v�Z����
	PIXVAL mixPixel(PIXVAL cols[]) const;
};
