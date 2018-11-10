// resizeobj.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"

#include "utils.h"
#include "paknode.h"
#include "ImgConverter.h"
#include "VhclConverter.h"
#include "TileConverter.h"

const char *my_signature = "/resizeobj";

class ResizeObj
{
public:
	ResizeObj()
	{
		m_tc.imgConverter(&m_ic);
	};

	void convertFilemask(std::string filemask) const;

	void antialias(int val){ m_ic.alpha(val); };
	void addonPrefix(std::string val){ m_addonPrefix = val; };
	void newExtension(std::string val){ m_newExt = val; };
	void headerRewriting(bool val){ m_headerRewriting = val;};
	void keepImageSize(bool val){ m_keepImageSize = val; };
	void lengthMode(LengthMode val){ m_vc.lengthMode(val); };
	void specialColorMode(SCConvMode val){ m_ic.specialColorMode(val); };
	void tileNoAnimation(bool val){ m_tc.noAnimation(val); };
private:
	ImgConverter m_ic;
	VhclConverter m_vc;
	TileConverter m_tc;
	std::string m_addonPrefix;
	bool m_headerRewriting;
	bool m_keepImageSize;
	std::string m_newExt;

	void convertFile(std::string filename) const;
	void convertPak(PakFile &pak) const;
	void convertAddon(PakNode *addon) const;
};

/// �A�h�I�����ύX�����̑ΏۊO�Ƃ���`���ꗗ.
const char *renameNodeExceptions[] = {
	"GOOD",
//	"SMOK",
};


bool isRenameNode(const char *name)
{
	for(int i = 0; i<lengthof(renameNodeExceptions); i++)
	{
		if(strncmp(renameNodeExceptions[i], name, NODE_NAME_LENGTH)==0)
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
void renameobj(PakNode *node, std::string prefix)
{
	if(!isRenameNode(node->type().data()))
		return;

	// XREF�m�[�h�̏ꍇ�͕ϊ�����
	if(node->type()=="XREF")
	{
		PakXRef *x = &node->data_p()->xref;

		if(isRenameNode(&x->type[0]))
		{
			if(node->data()->size()>sizeof(PakXRef))
			{
				std::vector<char> *dat = node->data();
				dat->insert(dat->begin() + offsetof(PakXRef, name),
					prefix.begin(), prefix.end()); 
			}
		}
	}	
	
	// �ŏ��̎q�m�[�h��TEXT�̏ꍇ�͂�����A�h�I�����Ƃ݂Ȃ��ĕϊ�����B
	if(node->begin() != node->end() && (*node->begin())->type()=="TEXT")
	{
		std::vector<char> *dat = (*node->begin())->data();
		dat->insert(dat->begin(), prefix.begin(), prefix.end()); 
	}

	for(PakNode::iterator it = node->begin(); it != node->end(); it++) renameobj(*it, prefix);
}


void ResizeObj::convertAddon(PakNode *addon) const
{
	std::string name = nodeStrings(addon->begin(), addon->end());
	printf("  �A�h�I�� -- %s:\n", name.c_str());

	if(m_keepImageSize)
	{
		if(addon->type() == "VHCL")
		{
			printf("    ��蕨�̉摜�\���ʒu�𒲐����܂��B\n");
			m_vc.convertVhclAddon(addon);
		}
		else if(addon->type() == "SMOK")
		{
			printf("    ���̉摜�\���ʒu�𒲐����܂��B\n");
			m_tc.convertAddon(addon);
		}
		else if(addon->type() == "BUIL" || addon->type() == "FACT"
			  || addon->type() == "FIEL")
		{
			printf("    ���z���̃^�C�������g�債�܂��B\n");
			m_tc.convertAddon(addon);
		}
	}else{
		printf("    �摜���k�����܂��B\n");
		m_ic.convertAddon(addon);
	}

	if(m_addonPrefix.size())
	{
		printf("    �A�h�I������ύX���܂��B\n");
		renameobj(addon, m_addonPrefix);
	}
}

void ResizeObj::convertPak(PakFile &pak) const
{
	if(m_headerRewriting)
		pak.signature(pak.signature() + my_signature);

	PakNode &root = pak.root();
	for(PakNode::iterator it = root.begin(); it != root.end(); it++)
		convertAddon(*it);
}

void ResizeObj::convertFile(std::string filename) const
{
	puts("====================================");
	printf("�t�@�C�� -- %s:\n", filename.c_str());

	PakFile pak;
	pak.loadFromFile(filename);

	if (pak.signature().find(my_signature)!=std::string::npos)
	{
		puts("  �X�L�b�v���܂���.");
		return;
	}

	convertPak(pak);

	if(m_newExt != "")
		filename = changeFileExt(filename, m_newExt);

#ifdef _DEBUG
	// �u-E=NUL�v�Ńt�@�C���o�͖���
	if(m_newExt == "NUL") return;
#endif
	pak.saveToFile(filename);
}

void ResizeObj::convertFilemask(std::string filemask) const
{
	if (filemask.find_first_of("*?")==std::string::npos)
	{
		convertFile(filemask);
	}else{
		std::vector<std::string> files;
		fileList(files, filemask);
		for(std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
			convertFile(*it);
	}
}


void printOption()
{
	puts(
#ifdef _DEBUG
	"DEBUG "
#endif
	"resizeobj ver 1.1.0 beta by wa\n\n"
	"resizeobj [�I�v�V����...] �Ώۃt�@�C���}�X�N...\n"
	"�I�v�V����:\n\n"
	" -A=�s0�`100�t�摜�k�����̃A���`�G�C���A�X��\n"
	"                0: �A���`�G�C���A�X����\n"
	"              100: �A���`�G�C���A�X����(����l)\n"
	" -S=�s0�`2�t  �摜�k�����̓���F(�����F�E�v���C���[�F)�̈���\n"
	"                0: ����F���g�p�����k�����A���Ԃ̌����ڂ�D�悷��\n"
	"                1: �k�����G���A�̍��オ����F�̏ꍇ�͂�����o��(����l)\n"
	"                2: �k�����G���A�œ���F�������ȏ�g�p����Ă���ꍇ�͂�����o��\n"
	"\n"
	" -K           �������[�h\n"
	" -KA          �������[�h�Ō��z����ϊ�����ۂɃA�j���[�V��������菜��\n"
	" -L           �������[�h�ŏ�蕨��ϊ�����ۂ�length������ϊ����Ȃ�\n"
	"\n"
	" -E=�s�g���q�t�o�͂���t�@�C���̊g���q�B����l�́u.64.pak�v\n"
	" -T=�stext�t  �A�h�I�����̐擪�Ɏw�肳�ꂽtext��ǉ�����\n"
	" -N           �t�@�C���w�b�_�̏����������s��Ȃ�\n"
	"\n"
	" -D           �G���[���Ƀ_�C�A���O��\�����Ȃ�\n"
	" -? , -H      ���̉�ʂ��o�͂���\n"
	"");
}

template<class T> T optToEnum(const std::string &text, int high, const char *opt)
{
	int value = atoi(text.c_str());
	if (value<0 || high<value)
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

	bool showErrorDialog = true;

	try
	{
		std::locale::global(std::locale(""));

		ResizeObj ro;
		ro.antialias(100);
		ro.headerRewriting(true);
		ro.lengthMode(STRICT_CONVERT_LENGTH);
		ro.specialColorMode(scmTOPLEFT);
		ro.addonPrefix("");
		ro.keepImageSize(false);
		ro.newExtension(".64.pak");
		ro.tileNoAnimation(false);

		std::vector<std::string> args;
		for(int i=1; i<argc; i++)
		{
			std::string key, val;
			parseArg(argv[i], key, val);
			if(key==""){ args.push_back(val);}
			else if(key=="K"){ ro.keepImageSize(true); }
			else if(key=="KA"){ ro.tileNoAnimation(true); }
			else if(key=="A"){ ro.antialias(optToEnum<int>(val, 100, "A"));}
			else if(key=="S"){ ro.specialColorMode(optToEnum<SCConvMode>(val, 2, "S")); }
			else if(key=="N"){ ro.headerRewriting(false);    }
			else if(key=="T"){ ro.addonPrefix(val);        }
			else if(key=="L"){ ro.lengthMode(NO_CONVERT_LENGTH); }
			else if(key=="E"){ if(val!="") ro.newExtension(val); }
			else if(key=="D"){ showErrorDialog = false; }
			else if(key=="H" || key=="?")
			{
				printOption();
				return 0;
			}
			else{
				printf("�����ȃX�C�b�`�ł� : \"%s\"\n\n", key.c_str());
				printOption();
				return 1;
			}
		}
		if(args.empty())
		{
			printOption();
			return 0;
		}

		for(std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++)
			ro.convertFilemask(*it);

	}catch(const std::exception &e)
	{
		printf("\n�G���[: %s\n", e.what());
		if(showErrorDialog)
			MessageBox(0, e.what(), "�G���[", MB_OK | MB_APPLMODAL | MB_ICONERROR);
		return 1;
	}		
	return 0;
}

