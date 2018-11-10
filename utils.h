#pragma once

#define lengthof(ary) (sizeof(ary)/sizeof(ary[0]))

/** �|�C���^�^����ʂ̃|�C���^�^�փL���X�g.
 * http://www.geocities.jp/ky_webid/cpp/language/024.html
 */
template <class T>
inline T pointer_cast(void* p)
{
	return static_cast<T>(p);
}
template <class T>
inline T pointer_cast(const void* p)
{
	return static_cast<T>(p);
}

/*
  �G���[�`�F�b�N�t���̃|�C���^��dynamic_cast
*/
template<class T, class U> 
inline T object_cast(U* o)
{
	T v = dynamic_cast<T>(o);
	if(!v && o) throw std::bad_cast();
	return v;
}

/**  �R�}���h���C���I�v�V��������͂���.
 *   options = (-|--|/)[A-Za-z90-9_]+([:=]?.+)?
 *   @param arg ��͑Ώ�
 *   @param[out] key   �I�v�V�����̏ꍇ�͑啶���ł��̃L�[�����A�t�@�C�����Ȃ�󕶎�����i�[
 *   @param[out] value �l����I�v�V�����̏ꍇ�͂���A�l���I�v�V�����Ȃ�󕶎���A�t�@�C�����Ȃ炻����i�[
 */
void parseArg(const char *arg, std::string &key, std::string &value);

/** �t�@�C�����g���q�̕ύX���s��.
 *
 *  @param path �ύX���B�ύX�����͍̂Ō��"."�ȍ~
 *  @param ext "."���܂߂Ďw�肷��B""���g�p����Ίg���q�̏����ƂȂ�B
 */
std::string changeFileExt(const std::string path, const std::string ext);

/** �f�B���N�g�������̃t�@�C���ꗗ���쐬����. */
void fileList(std::vector<std::string> &entries, const std::string mask);
