// resizeobj.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"

#include <fcntl.h>
#include <io.h>

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

	void convertStdIO() const;
	void convertFile(std::string filename) const;

	void antialias(int val){ m_ic.alpha(val); };
	void addonPrefix(std::string val){ m_addonPrefix = val; };
	void newExtension(std::string val){ m_newExt = val; };
	void headerRewriting(bool val){ m_headerRewriting = val;};
	void keepImageSize(bool val){ m_keepImageSize = val; };
	void specialColorMode(SCConvMode val){ m_ic.specialColorMode(val); };
	void tileNoAnimation(bool val){ m_tc.noAnimation(val); };
	void newTileSize(int val){ m_ic.newTileSize(val); };
private:
	ImgConverter m_ic;
	VhclConverter m_vc;
	TileConverter m_tc;
	std::string m_addonPrefix;
	bool m_headerRewriting;
	bool m_keepImageSize;
	std::string m_newExt;

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


std::string addonName(PakNode *node)
{
	if(node->type()=="TEXT")
		return node->data_p()->text;
	else if(node->length()>0)
		return addonName((*node)[0]);
	else	
		return "";
}



void ResizeObj::convertAddon(PakNode *addon) const
{
	std::clog << "    " << addonName(addon) << std::endl;

	if(m_keepImageSize)
	{
		if(addon->type() == "VHCL")
		{
			m_vc.convertVhclAddon(addon, m_ic.newTileSize());
		}
		else if(addon->type() == "SMOK")
		{
			m_tc.convertAddon(addon);
		}
		else if(addon->type() == "BUIL" || addon->type() == "FACT"
			  || addon->type() == "FIEL")
		{
			m_tc.convertAddon(addon);
		}
	}else{
		m_ic.convertAddon(addon);
	}

	if(m_addonPrefix.size())
	{
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

	if (pak.signature().find(my_signature)!=std::string::npos)
	{
		std::clog << "    skipped." << std::endl;
		return;
	}

	convertPak(pak);

	if(m_newExt != "") filename = changeFileExt(filename, m_newExt);

	pak.saveToFile(filename);
}

void printOption()
{
	std::clog << 
#ifdef _DEBUG
	"DEBUG "
#endif
	"resizeobj ver 1.2.0 beta by wa\n\n"
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
	" -K           �������[�h\n"
	" -KA          �������[�h�Ō��z����ϊ�����ۂɃA�j���[�V��������菜��\n"
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

	bool isShowErrorDialog = true;

	try
	{
		std::locale::global(std::locale(""));

		ResizeObj ro;
		ro.antialias(100);
		ro.headerRewriting(true);
		ro.specialColorMode(scmTOPLEFT);
		ro.addonPrefix("");
		ro.keepImageSize(false);
		ro.newExtension(".64.pak");
		ro.tileNoAnimation(false);

		std::vector<std::string> files;
		for(int i=1; i<argc; i++)
		{
			std::string key, val;
			parseArg(argv[i], key, val);
			if(key==""){
				if (val.find_first_of("*?")==std::string::npos)
					files.push_back(val);
				else
					fileList(files, val);
			}
			else if(key=="K"){ ro.keepImageSize(true); }
			else if(key=="KA"){ ro.tileNoAnimation(true); }
			else if(key=="A"){ ro.antialias(optToEnum<int>(val, 100, "A"));}
			else if(key=="S"){ ro.specialColorMode(optToEnum<SCConvMode>(val, 2, "S")); }
			else if(key=="W"){
				int ts = optToEnum<int>(val, 255, "W");
				if(ts%8 != 0) throw std::runtime_error("�^�C���T�C�Y��8�̔{���Ɍ���܂��B"); 
				ro.newTileSize(ts);
			}
			else if(key=="N"){ ro.headerRewriting(false);    }
			else if(key=="T"){ ro.addonPrefix(val);        }
			else if(key=="E"){ if(val!="") ro.newExtension(val); }
			else if(key=="D"){ isShowErrorDialog = false; }
			else if(key=="H" || key=="?")
			{
				printOption();
				return 0;
			}
			else{
				std::clog << "�����ȃI�v�V�����ł� : " << key << std::endl;
				printOption();
				return 1;
			}
		}
		if(files.empty())
		{
			printOption();
			return 0;
		}

		for(std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
		{
			if(*it == "con")
				ro.convertStdIO();
			else
				ro.convertFile(*it);
		}

	}catch(const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
		if(isShowErrorDialog)
			showErrorDialog(e.what(), "Error");
		return 1;
	}		
	return 0;
}

