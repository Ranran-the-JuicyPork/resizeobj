#pragma once

#include "paknode.h"

enum LengthMode{
	NO_CONVERT_LENGTH,
	CONVERT_LENGTH,
	STRICT_CONVERT_LENGTH
};

class VhclConverter
{
public:
	static const int OFFSET_X = 32;
	static const int OFFSET_Y = 44;
	/** VHCL�m�[�h��length�̔{���E�q�m�[�h�̃I�t�Z�b�g�̒��߂��s��. */
	void convertVhclAddon(PakNode *node) const;

	void lengthMode(LengthMode val){ m_lengthMode = val; };
protected:
	void convertImage(PakNode *node, int index) const;
	void convertImageList(PakNode *node) const;
	void convertImageList2(PakNode *node) const;
private:
	LengthMode m_lengthMode;
};
