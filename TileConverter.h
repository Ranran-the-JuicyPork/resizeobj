#pragma once

#include "paknode.h"
#include "SimuImage.h"
#include "ShrinkConverter.h"

class TileConverter
{
private:
	bool m_noAnimation;
	ImgConverter *m_ic;
protected:
	void convertNodeTree(PakNode *node) const;
	void convertFactorySmoke(PakNode *node) const;
	void convertBuil(PakNode *node) const;
	void convertTile(int layout, int x, int y,
		PakNode *tiles[], PakNode *srcTile, int width, int height) const;
	void convertImg2(int x, int y, PakNode *img2s[], PakNode *srcImg2, int phasen) const;
	void encodeImg2(PakNode *img2, std::vector<Bitmap<PIXVAL>*> &bitmaps,
		int bx, int by, int maxHeight, int width, int height, int version) const;

	void convertSmokeTreeImage(PakNode *node) const;
public:
	// �A�j���[�V�����𖳌�������
	void setNoAnimation(bool val) { m_noAnimation = val; };
	bool noAnimation() { return m_noAnimation; }
	// �J�[�\���摜�E�t�B�[���h�k���p
	void imgConverter(ImgConverter *ic) { m_ic = ic; };

	// �A�h�I����ϊ�����
	void convertAddon(PakNode *node) const { convertNodeTree(node); };
};

/// bmp�̗̈�[x,y, +width, + height]��SIMU_TRANSPARENT�ȊO�̃h�b�g������ΐ^��Ԃ��B
bool hasOpaquePixel(const Bitmap<PIXVAL> &bmp, int x, int y, int width, int height);

/// bmp�̗̈�[x,y  +width, +height*maxHeight]�����ԏ�̔�󔒃}�X��T���āA�^�C���̍��������肷��B
/// �S�ċ󔒃}�X�̏ꍇ�Ȃ�1�ƂȂ�B
int getTileHeight(const Bitmap<PIXVAL> &bmp, int x, int y, int tileWidth, int tileHeight, int maxHeight);