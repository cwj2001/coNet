//
// Created by 抑~风 on 2023/2/6.
//

#ifndef CWJ_CO_NET_BYTEARRAY_H
#define CWJ_CO_NET_BYTEARRAY_H
#include <inttypes.h>
#include <cstddef>
#include <vector>
#include <sys/uio.h>
#include <memory>

namespace CWJ_CO_NET{

    class ByteArray{
    public:

        using ptr = std::shared_ptr<ByteArray>;

        struct Node{
            Node(size_t mSize,Node* next = nullptr);
            virtual ~Node();
            char * m_ptr;
            Node * m_next;
            size_t m_size;
        };

        ByteArray(size_t mBaseSize, size_t mCapacity = 0);

        void updateCapacity(size_t size);

        size_t getMCapacity() const;

        size_t getMDataSize() const;

        size_t getMBaseSize() const;

        void write(const std::string& buf);

        const std::string read(size_t size);

        size_t write(const char * buf,int size);

        size_t read(char * buf,int size);

        size_t getWriteBuffers(std::vector<iovec>& buffers, size_t len,bool is_update=true);

        size_t getReadBuffers(std::vector<iovec>& buffers, size_t len = ~0ull,bool is_update=true);

        void updateNullRead(size_t r_size);
        void updateNullWrite(size_t r_size);

    private:
        void deallocNode(Node *node);
    private:
        size_t m_base_size{0};
        size_t m_capacity{0};
        size_t m_data_size{0};

        size_t m_write_index{0};
        Node* m_write_node{nullptr};

        size_t m_read_index{0};
        Node* m_read_node{nullptr};

        Node* m_end_node{nullptr};



    };

}

#endif //CWJ_CO_NET_BYTEARRAY_H
