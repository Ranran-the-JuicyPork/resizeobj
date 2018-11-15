// resizeobj.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"

#include <fcntl.h>
#include <io.h>

#include "utils.h"
#include "paknode.h"
#include "ImgConverter.h"
#include "ImgUpscaleConverter.h"
#include "TileConverter.h"

const char *RESIZEOBJ_SIGNATURE = "/resizeobj";

enum ConvertMode
{
	cmNoConvert,
	cmDownscale,
	cmUpscale,
	cmSplit,
};

class ResizeObj
{
private:
	ImgConverter m_ic;
	ImgUpscaleConverter m_iuc;
	TileConverter m_tc;
	std::string m_addonPrefix;
	bool m_headerRewriting;
	ConvertMode m_convertMode;
	std::string m_newExt;

	void convertPak(PakFile &pak) const;
	void convertAddon(PakNode *addon) const;
public:
	ResizeObj();
	void convertStdIO() const;
	void convertFile(std::string filename) const;

	void setAntialias(int val) { m_ic.setAlpha(val); };
	void setAddonPrefix(std::string val) { m_addonPrefix = val; };
	void setNewExtension(std::string val) { m_newExt = val; };
	void setHeaderRewriting(bool val) { m_headerRewriting = val; };
	void setConvertMode(ConvertMode val) { m_convertMode = val; };
	void setSpecialColorMode(SCConvMode val) { m_ic.setSpecialColorMode(val); };
	void setTileNoAnimation(bool val) { m_tc.setNoAnimation(val); };
	void setNewTileSize(int val) { m_ic.setNewTileSize(val); };
};

/// �A�h�I�����ύX�����̑ΏۊO�Ƃ���`���ꗗ.
const char *RENAME_NODE_EXCEPTIONS[] = {
	"GOOD",
	//	"SMOK",
};


bool isRenameNode(const char *name)
{
	for (int i = 0; i < lengthof(RENAME_NODE_EXCEPTIONS); i++)
	{
		if (strncmp(RENAME_NODE_EXCEPTIONS[i], name, NODE_NAME_LENGTH) == 0)
			return false;
	}
	return true;
}

/**
	�A�h�I�����̐擪��posfix��ǉ�����.

	�A�h�I������`�p��TEXT�m�[�h�ƃA�h���Q�Ɨp��XREF�m�[�h�̐擪��prefix��ǉ�����B
����:
pak64:  Buecher
pak128: Bucher
*/
void renameObj(PakNode *node, std::string prefix)
{
	if (!isRenameNode(node->type().data()))
		return;

	// XREF�m�[�h�̏ꍇ�͕ϊ�����
	if (node->type() == "XREF")
	{
		PakXRef *x = &node->dataP()->xref;

		if (isRenameNode(&x->type[0]))
		{
			if (node->data()->size() > sizeof(PakXRef))
			{
				std::vector<char> *dat = node->data();
				dat->insert(dat->begin() + offsetof(PakXRef, name),
					prefix.begin(), prefix.end());
			}
		}
	}

	// �ŏ��̎q�m�[�h��TEXT�̏ꍇ�͂�����A�h�I�����Ƃ݂Ȃ��ĕϊ�����B
	if (node->begin() != node->end() && (*node->begin())->type() == "TEXT")
	{
		std::vector<char> *dat = (*node->begin())->data();
		dat->insert(dat->begin(), prefix.begin(), prefix.end());
	}

	for (PakNode::iterator it = node->begin(); it != node->end(); it++)
		renameObj(*it, prefix);
}

std::string getAddonName(PakNode *node)
{
	if (node->type() == "TEXT")
		return node->dataP()->text;
	else if (node->length() > 0)
		return getAddonName(node->at(0));
	else
		return "";
}

ResizeObj::ResizeObj()
{
	m_tc.imgConverter(&m_ic);
}

void ResizeObj::convertAddon(PakNode *addon) const
{
	std::clog << "    " << getAddonName(addon) << std::endl;

	switch (m_convertMode)
	{
	case cmSplit:
		if (addon->type() == "SMOK")
		{
			m_tc.convertAddon(addon);
		}
		else if (addon->type() == "BUIL" || addon->type() == "FACT"
			|| addon->type() == "FIEL")
		{
			m_tc.convertAddon(addon);
		}
		break;

	case cmDownscale:
		m_ic.convertAddon(addon);
		break;

	case cmUpscale:
		m_iuc.convertAddon(addon);
		break;
	}

	if (m_addonPrefix.size())
	{
		renameObj(addon, m_addonPrefix);
	}
}

void ResizeObj::convertPak(PakFile &pak) const
{
	if (m_headerRewriting)
		pak.setSignature(pak.signature() + RESIZEOBJ_SIGNATURE);

	PakNode &root = pak.root();
	for (PakNode::iterator it = root.begin(); it != root.end(); it++)
		convertAddon(*it);
}

void ResizeObj::convertStdIO() const
{
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	PakFile pak;
	pak.load(std::cin);
	convertPak(pak);
	pak.save(std::cout);
}

void ResizeObj::convertFile(std::string filename) const
{
	std::clog << filename << std::endl;


	PakFile pak;
	pak.loadFromFile(filename);

	if (pak.signature().find(RESIZEOBJ_SIGNATURE) != std::string::npos)
	{
		std::clog << "    skipped." << std::endl;
		return;
	}

	convertPak(pak);

	if (m_newExt != "") filename = changeFileExtension(filename, m_newExt);

	pak.saveToFile(filename);
}

void printOption()
{
	std::clog <<
#ifdef _DEBUG
		"DEBUG "
#endif
		"resizeobj ver 1.5.0 beta by wa\n\n"
		"resizeobj [�I�v�V����...] �Ώۃt�@�C���}�X�N...\n"
		"�I�v�V����:\n\n"
		" -A=(0...100) �摜�k�����̃A���`�G�C���A�X��\n"
		"                0: �A���`�G�C���A�X����\n"
		"              100: �A���`�G�C���A�X����(����l)\n"
		" -S=(0...2)   �摜�k�����̓���F(�����F�E�v���C���[�F)�̈���\n"
		"                0: ����F���g�p�����k�����A���Ԃ̌����ڂ�D�悷��\n"
		"                1: �k�����G���A�̍��オ����F�̏ꍇ�͂�����o��(����l)\n"
		"                2: �k�����G���A�œ���F�������ȏ�g�p����Ă���ꍇ�͂�����o��\n"
		" -W=(16...255) �摜�ϊ���̃^�C���T�C�Y���w�肷��B����l�́u64�v"
		"\n"
		" -K           �����僂�[�h\n"
		" -KA          �����僂�[�h�Ō��z����ϊ�����ۂɃA�j���[�V��������菜��\n"
		"\n"
		" -X           �g�僂�[�h\n"
		"\n"
		" -E=(ext)     �o�͂���t�@�C���̊g���q�B����l�́u.64.pak�v\n"
		" -T=(text)    �A�h�I�����̐擪�Ɏw�肳�ꂽtext��ǉ�����\n"
		" -N           �t�@�C���w�b�_�̏����������s��Ȃ�\n"
		"\n"
		" -D           �G���[���Ƀ_�C�A���O��\�����Ȃ�\n"
		" -? , -H      ���̉�ʂ��o�͂���"
		<< std::endl;
}

template<class T> T optToEnum(const std::string &text, int high, const char *opt)
{
	int value = atoi(text.c_str());
	if (value < 0 || high < value)
	{
		std::ostringstream os;
		os << opt << "�I�v�V�����̗L���͈͂�0�`" << high << "�ł��B: " << text;
		throw std::runtime_error(os.str());
	}
	return static_cast<T>(value);
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	bool isShowErrorDialog = true;

	try
	{
		//std::locale::global(std::locale(""));

		ResizeObj ro;
		ro.setAntialias(100);
		ro.setHeaderRewriting(true);
		ro.setSpecialColorMode(scmTOPLEFT);
		ro.setAddonPrefix("");
		ro.setConvertMode(cmDownscale);
		ro.setNewExtension(".64.pak");
		ro.setTileNoAnimation(false);

		std::vector<std::string> files;
		for (int i = 1; i < argc; i++)
		{
			std::string key, val;
			parseArg(argv[i], key, val);
			if (key == "")
			{
				if (val.find_first_of("*?") == std::string::npos)
					files.push_back(val);
				else
					fileList(files, val);
			}
			else if (key == "K")
			{
				ro.setConvertMode(cmSplit);
			}
			else if (key == "X")
			{
				ro.setConvertMode(cmUpscale);
				ro.setNewExtension(".128.pak");
			}
			else if (key == "KA")
			{
				ro.setTileNoAnimation(true);
			}
			else if (key == "A")
			{
				ro.setAntialias(optToEnum<int>(val, 100, "A"));
			}
			else if (key == "S")
			{
				ro.setSpecialColorMode(optToEnum<SCConvMode>(val, 2, "S"));
			}
			else if (key == "W")
			{
				int ts = optToEnum<int>(val, 0xFFFF, "W");
				if (ts % 8 != 0) throw std::runtime_error("�^�C���T�C�Y��8�̔{���Ɍ���܂��B");
				ro.setNewTileSize(ts);
			}
			else if (key == "N")
			{
				ro.setHeaderRewriting(false);
			}
			else if (key == "T")
			{
				ro.setAddonPrefix(val);
			}
			else if (key == "E")
			{
				if (val != "") ro.setNewExtension(val);
			}
			else if (key == "D")
			{
				isShowErrorDialog = false;
			}
			else if (key == "H" || key == "?")
			{
				printOption();
				return 0;
			}
			else
			{
				std::clog << "�����ȃI�v�V�����ł� : " << key << std::endl;
				printOption();
				return 1;
			}
		}
		if (files.empty())
		{
			printOption();
			return 0;
		}

		for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
		{
			if (*it == "con")
				ro.convertStdIO();
			else
				ro.convertFile(*it);
		}

	}
	catch (const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
		if (isShowErrorDialog)
			showErrorDialog(e.what(), "Error");
		return 1;
	}
	return 0;
}

