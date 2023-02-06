//
// Created by 抑~风 on 2023/2/6.
//

#include <cstring>
#include <string>
#include <memory>
#include "byteArray.h"
#include "log.h"
#include "macro.h"

namespace CWJ_CO_NET {
    static auto g_logger = GET_LOGGER("system");
    CWJ_CO_NET::ByteArray::Node::Node(size_t mSize, Node *next) : m_size(mSize), m_next(next) {
        m_ptr = new char[mSize];
    }

    CWJ_CO_NET::ByteArray::Node::~Node() {
        delete[] m_ptr;
    }

    CWJ_CO_NET::ByteArray::ByteArray(size_t mBaseSize, size_t mCapacity)
            : m_base_size(mBaseSize) {

        Node *head_node = new Node(m_base_size);
        m_capacity = m_base_size;
        m_read_index = 0;
        m_read_node = head_node;
        m_write_index = 0;
        m_write_node = head_node;
        m_end_node = head_node;
        m_data_size = 0;

        updateCapacity(mCapacity);

    }

    void CWJ_CO_NET::ByteArray::updateCapacity(size_t size) {
        size_t old_size = m_capacity;
        while (size >= m_capacity) {
            Node *new_node = new Node(m_base_size);
            m_end_node->m_next = new_node;
            m_end_node = new_node;
            m_capacity += m_base_size;
        }
    }

    size_t CWJ_CO_NET::ByteArray::getMCapacity() const {
        return m_capacity;
    }

    size_t CWJ_CO_NET::ByteArray::getMDataSize() const {
        return m_data_size;
    }

    size_t CWJ_CO_NET::ByteArray::getMBaseSize() const {
        return m_base_size;
    }

    size_t CWJ_CO_NET::ByteArray::write(const char *buf, int size) {
        updateCapacity(size);
        size_t w_size = 0;
        while (size > 0) {
            CWJ_ASSERT(m_write_node);
            if (m_base_size - m_write_index > size) {
                if(buf)memcpy(m_write_node->m_ptr + m_write_index, buf+w_size, size);

                m_write_index += size;
                w_size += size;
                size = 0;

            } else {

                size_t rt = size - (m_base_size - m_write_index);
                if(buf)memcpy(m_write_node->m_ptr + m_write_index, buf+w_size, (m_base_size - m_write_index));

                w_size += (m_base_size - m_write_index);
                size = rt;

                m_write_index = 0;
                CWJ_ASSERT(m_write_node->m_next);
                m_write_node = m_write_node->m_next;
            }

        }
        m_data_size += w_size;
        m_capacity -= w_size;
        return w_size;
    }

    size_t CWJ_CO_NET::ByteArray::read(char *buf, int size) {
        size = m_data_size > size ? size : m_data_size;
        size_t r_size = 0;
        while (size > 0) {
            if (m_base_size - m_read_index > size) {

                if(buf) memcpy(buf+r_size, m_read_node->m_ptr + m_read_index, size);

                m_read_index += size;
                r_size += size;
                size = 0;
            } else {
                size_t rt = size - (m_base_size - m_read_index);

                if(buf)memcpy(buf+r_size, m_read_node->m_ptr + m_read_index, (m_base_size - m_read_index));

                size = rt;
                r_size += (m_base_size - m_read_index);

                m_read_index = 0;
                auto tmp = m_read_node;
                m_read_node = m_read_node->m_next;
                deallocNode(tmp);
            }
        }
        m_data_size -= r_size;
        return r_size;
    }

    void CWJ_CO_NET::ByteArray::write(const std::string &buf) {
        write(buf.c_str(), buf.size());
    }

    const std::string CWJ_CO_NET::ByteArray::read(size_t size) {
        std::shared_ptr<char> buf(new char[size+1], [](auto &a) { delete[] a; });
        read(buf.get(), size);
        return std::string(buf.get(),0,size);
    }

    size_t CWJ_CO_NET::ByteArray::getWriteBuffers(std::vector<iovec> &buffers, size_t size,bool is_update) {
//        CWJ_ASSERT(m_write_node == m_read_node);
//        CWJ_ASSERT(m_write_index == m_read_index);
        updateCapacity(size);
        size_t w_size = size;

        size_t write_index = m_write_index;
        Node* write_node = m_write_node;

        while (size > 0) {
            iovec io_buf;
            if (m_base_size - write_index > size) {
                io_buf.iov_base = write_node->m_ptr + write_index;
                io_buf.iov_len = size;

                write_index += size;
                size = 0;
            } else {
                size_t rt = size - (m_base_size - write_index);

                io_buf.iov_base = write_node->m_ptr + write_index;
                io_buf.iov_len = (m_base_size - write_index);

                size = rt;

                auto tmp = write_node;
                write_index = 0;
                write_node = write_node->m_next;
            }
            buffers.push_back(io_buf);
        }
        if(is_update) {
            m_write_node = write_node;
            m_write_index = write_index;
            m_data_size += w_size;
            m_capacity -= w_size;
        }
        return w_size;
    }

    void ByteArray::updateNullWrite(size_t w_size) {
       write(nullptr,w_size);
    }

    size_t CWJ_CO_NET::ByteArray::getReadBuffers(std::vector<iovec> &buffers, size_t size,bool is_update) {
        size = m_data_size > size ? size : m_data_size;
        size_t r_size = size;

        size_t read_index = m_read_index;
        Node* read_node = m_read_node;

        while (size > 0) {
            iovec io_buf;
            if (m_base_size - read_index > size) {
                io_buf.iov_base = read_node->m_ptr + read_index;
                io_buf.iov_len = size;

                read_index += size;
                size = 0;


            } else {
                size_t rt = size - (m_base_size - read_index);

                io_buf.iov_base = read_node->m_ptr + read_index;
                io_buf.iov_len = (m_base_size - read_index);

                size  = rt;

                read_index = 0;
                Node *tmp = read_node;
                read_node = read_node->m_next;
                if(is_update) deallocNode(tmp);
            }
            buffers.push_back(io_buf);
        }
        if(is_update) {
            m_read_node = read_node;
            m_read_index = read_index;
            m_data_size -= r_size;
        }
        return r_size;
    }

    void ByteArray::updateNullRead(size_t r_size){
        read(nullptr,r_size);
    }

    void CWJ_CO_NET::ByteArray::deallocNode(CWJ_CO_NET::ByteArray::Node *node) {
        node->m_next = nullptr;
        m_end_node->m_next = node;
        m_end_node = node;
        m_capacity += node->m_size;
    }


}
