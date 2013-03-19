#ifndef _LOCKLESS_QUEUE_
#define _LOCKLESS_QUEUE_

#include <stdint.h>
#include <cstring>
#include <vector>

namespace aq 
{

	/*!
	* \brief Lockless queue for one thread pushing and one thread poping
	*/
	template <class T> class LocklessQueue
	{
	private:
		std::vector<T> queue;
		size_t head;
		size_t tail;

	public:
		LocklessQueue(size_t size)
			: queue(size + 1), 
			head(0), 
			tail(0)
		{
		}

		bool push(T data)
		{
			size_t h = this->head;

			if ((h+1) % this->queue.capacity() == this->tail)
				return false;

			this->queue[h++] = data;

			if(h == this->queue.capacity())
				h = 0;

			this->head = h;
			return true;
		}

		bool pop(T* data)
		{
			size_t t = this->tail;

			if(t == this->head)
				return false;

			*data = this->queue[t++];

			if(t == this->queue.capacity())
				t = 0;

			this->tail = t;
			return true;
		}

		bool empty() const
		{
			return this->tail == this->head;
		}

		bool full() const
		{
			return ((this->head+1) % this->queue.capacity()) == this->tail;
		}

		size_t getSize() const
		{
			return this->queue.capacity();
		}

		/*! \brief Get remaind empty size
		*
		* Warning, this is just an estimation, because this function is not thread safe.
		*/
		size_t getRemaindSize() const
		{
			size_t t = this->tail;
			size_t h = (this->head + 1) % this->queue.capacity();

			if      (h > t) return this->queue.capacity() - (h - t);
			else if (h < t) return t - h;
			else            return 0; // head == tail
		}

	};

}

#endif
