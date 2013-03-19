#pragma once

#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/timer/timer.hpp>

namespace
{
	template <typename T>
	struct Process
	{
	public:
		Process(T v[], boost::uint64_t size, boost::function<void (T, unsigned)> cb, unsigned threadId)
			: m_v(v), m_size(size), m_cb(cb), m_threadId(threadId)
		{
		}
		void operator()()
		{
			for (boost::uint64_t i = 0; i < m_size; ++i)
			{
				m_cb(m_v[i], m_threadId);
			}
		}
	private:
		T * m_v; 
		boost::uint64_t m_size; 
		boost::function<void (T, unsigned)> m_cb;
		const std::string m_indent;
		unsigned m_threadId;
	};

	template <typename T>
	struct ProcessVector
	{
	public:
		ProcessVector(typename std::vector<T>::const_iterator b, 
			typename std::vector<T>::const_iterator e, 
			boost::function<void (T, unsigned)> cb,
			unsigned threadId)
			: m_b(b), m_e(e), m_cb(cb), m_threadId(threadId)
		{
		}

		void operator()()
		{
			boost::uint64_t value = 0;
			for (vint_t::const_iterator it = m_b; it != m_e; ++it)
			{
				m_cb(*it, m_threadId);
			}
		}
	private:
		typename std::vector<T>::const_iterator m_b;
		typename std::vector<T>::const_iterator m_e; 
		boost::function<void (T, unsigned)> m_cb; 
		const std::string m_indent;
		unsigned m_threadId;
	};

}

template <typename T>
int ParallelProcess(T v[], boost::uint64_t size, boost::function<void (T, unsigned)> cb, unsigned int nbProc)
{
	boost::thread_group grp;
	boost::uint64_t begin = 0;
	boost::uint64_t chunk = size / nbProc;
	for (boost::uint64_t i = 0; (i < nbProc); i++)
	{	
		typename Process<T> p(v + begin, chunk, cb, (unsigned)i);
		boost::thread * t = new boost::thread(p);
		grp.add_thread(t);
		begin += chunk;
	}

	grp.join_all();
	return 0;
}

template <typename T>
int ParallelProcess(std::vector<T>& v, boost::function<void (T, unsigned)> cb, unsigned int nbProc)
{
	boost::thread_group grp;
	typename std::vector<T>::const_iterator b = v.begin();
	typename std::vector<T>::const_iterator e = v.begin();
	for (boost::uint64_t i = 0; (i < nbProc) && (e != v.end()); i++)
	{	
		std::advance(e, v.size() / (unsigned int)nbProc);
		typename ProcessVector<T> pv(b, e, cb, (unsigned)i);
		boost::thread * t = new boost::thread(pv);
		grp.add_thread(t);
		b = e;
	}
	grp.join_all();
	return 0;
}