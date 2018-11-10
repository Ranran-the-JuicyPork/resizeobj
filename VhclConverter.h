#pragma once

#include "paknode.h"

class VhclConverter
{
public:
	static const int OFFSET_X = 32;
	static const int OFFSET_Y = 44;
	/** VHCL�m�[�h��length�̔{���E�q�m�[�h�̃I�t�Z�b�g�̒��߂��s��. */
	void convertVhclAddon(PakNode *node, int targetTileSize) const;
protected:
	void convertImage(PakNode *node, int index, int targetTileSize, int waytype) const;
	void convertImageList(PakNode *node, int targetTileSize, int waytype) const;
	void convertImageList2(PakNode *node, int targetTileSize, int waytype) const;
};
