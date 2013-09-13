// expandable_array.h: ���/�����Ԫ�س��Ȳ���������.
// bayou@tencent.com
// 2010-09-04
// ////////////////////////////////////////////////////////////////////
#ifndef COMMON_BASELIB_SVRPUBLIB_EXPANDABLE_ARRAY_H_
#define COMMON_BASELIB_SVRPUBLIB_EXPANDABLE_ARRAY_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

// -count(4B)-, |--len(4B)--|--data(len)--|--len(4B)--|--data--|....


//
// class:       VarDataSerialize
// description: package expandable array
//

class VarDataSerialize {
public:
    VarDataSerialize();

    ~VarDataSerialize();

    //  ��ոö���
    bool ResetContent();

    //  �����ڴ�����Ӳ���
    void SetExtendStep(uint32_t extend_step);

    //  ����һ�����ݵ��䳤������
    //  ������� data :   �ö����ݵĵ�ַ
    //  �������  len :   �ö����ݵĳ���
    //  ����ֵ�� �����Ƿ�ɹ�
    bool AddVarData(const char* data, uint32_t len);

    //  �õ��䳤��������л��ṹ
    //  ������� data :           �ö����ݵĵ�ַ
    //  �������  len :           �ö����ݵĳ���
    //  �������  item_count :    �ö����ݵ�Ԫ�ظ���
    //  ����ֵ�� ��ȡ�Ƿ�ɹ�
    bool GetPackage(const char** data,
                    uint32_t* len,
                    uint32_t* item_count = 0);

private:
    BufferV m_buff;

    //  �Ƿ��ѳ�ʼ��m_buff.buff��ͷ4�ֽ�Ϊitem_count
    bool m_is_inited;
};

class VarDataUnSerialize {
public:

    VarDataUnSerialize();

    ~VarDataUnSerialize();

    //  ��һ��������Ϊ��Ҫ�����л�������Դ
    //  ������� data :   �ö����ݵĵ�ַ
    //  �������  len :   �ö����ݵĳ���
    //  ����ֵ�� �����Ƿ�ɹ�
    bool AttachPackage(const char* data, uint32_t len);

    //  ��ȡ��һ���䳤Ԫ�ص�����
    //  ������� data :           ��Ԫ�صĵ�ַ
    //  �������  len :           ��Ԫ�ص����ݳ���
    //  �������  pos :           ��Ԫ���������
    //  ����ֵ�� ��ȡ�Ƿ�ɹ�
    bool GetNextVal(const char** data, uint32_t* len, uint32_t* pos = 0);

    //  ��ȡ�ö���������Ԫ�ص���Ŀ
    //  ����ֵ�� �ö���������Ԫ�ص���Ŀ
    uint32_t GetValidItemsCount() const;

private:
    const char*    m_data;              // ������ʼ��ַ
    uint32_t       m_valid_len;         // �䳤���е���ЧԪ�ظ���
    const char*    m_cursor;            // ��ǰָ����������ݵĵ�ַ
    uint32_t       m_current_pos;       // ��ǰָ����������ݵ����
    uint32_t       m_package_len;       // ��¼Attach�����ݰ����ȡ�
    // ��ֹGetNextVal()Խ��
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_EXPANDABLE_ARRAY_H_

