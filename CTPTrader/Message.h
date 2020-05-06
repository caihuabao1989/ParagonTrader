#pragma once
#include <mutex>
#include <queue>

struct Done{};

struct Message_Base
{
	virtual ~Message_Base() {}
};

template<typename Msg>
struct Wrapped_Message :public Message_Base
{
	Msg contents;
	explicit Wrapped_Message(const Msg& contents_) :contents(contents_) {}
};

class Queue
{
	std::mutex mut;
	std::condition_variable cond;
	std::queue<std::shared_ptr<Message_Base>> q;
public:
	template<typename T>
	void push(const T& msg_)
	{
		std::lock_guard<std::mutex> lk(mut);
		q.push(std::make_shared<Wrapped_Message<T>>(msg_));
		cond.notify_all();
	}

	std::shared_ptr<Message_Base> wait_and_pop()
	{
		std::unique_lock<std::mutex> lk(mut);
		cond.wait(lk, [&] {return !q.empty(); });
		auto res = q.front();
		q.pop();
		return res;
	}
};

class Sender
{
	mutable Queue* q;
public:
	friend bool operator<(const Sender& lhs, const Sender& rhs);
	Sender() :q(nullptr) {}
	explicit Sender(Queue* q_) :q(q_) {}
	template<typename Msg>
	void send(const Msg& msg_) const
	{
		if (q) q->push(msg_);
	}
};

bool operator<(const Sender& lhs, const Sender& rhs);


class Dispatcher
{
	Queue* q;
	bool chained;

	Dispatcher(const Dispatcher&) = delete;
	Dispatcher& operator=(const Dispatcher&) = delete;

	template<typename dispatcher, typename msg, typename func>
	friend class TemplateDispatcher;

	void wait_and_dispatch()
	{
		while (true)
		{
			auto msg = q->wait_and_pop();
			dispatch(msg);
		}
	}

	bool dispatch(const std::shared_ptr<Message_Base>& msg_)
	{
		auto ptr = dynamic_cast<Wrapped_Message<Done>*>(msg_.get());
		if (ptr)
		{
			throw Done();
		}
		return false;
	}

public:
	Dispatcher(Dispatcher&& other) noexcept :q(other.q), chained(other.chained) { other.chained = true; }
	explicit Dispatcher(Queue* q_) :q(q_), chained(false) {}

	template<typename msg, typename func>
	TemplateDispatcher<Dispatcher, msg, func> handle(func&& f)
	{
		return TemplateDispatcher<Dispatcher, msg, func>(q, this, std::forward<func>(f));
	}

	~Dispatcher() noexcept(false)
	{
		if (!chained)
			wait_and_dispatch();
	}
};

template<typename prevdispatcher, typename msg, typename func>
class TemplateDispatcher
{
	Queue* q;
	prevdispatcher* prev;
	func f;
	bool chained;

	TemplateDispatcher(const TemplateDispatcher&) = delete;
	TemplateDispatcher& operator=(const TemplateDispatcher&) = delete;

	template<typename dispatcher, typename othermsg, typename otherfunc>
	friend class TemplateDispatcher;

	void wait_and_dispatch()
	{
		while (true)
		{
			auto _msg = q->wait_and_pop();
			if (dispatch(_msg)) break;
		}
	}

	bool dispatch(const std::shared_ptr<Message_Base>& msg_)
	{
		if (Wrapped_Message<msg>* wrapper = dynamic_cast<Wrapped_Message<msg>*>(msg_.get()))
		{
			f(wrapper->contents);
			return true;
		}
		else
			return prev->dispatch(msg_);
	}

public:
	TemplateDispatcher(TemplateDispatcher&& other) :
		q(other.q), prev(other.prev), f(std::move(other.f)), chained(other.chained)
	{
		other.chained = true;
	}

	TemplateDispatcher(Queue* q_, prevdispatcher* prev_, func&& f_) :
		q(q_), prev(prev_), f(std::forward<func>(f_)), chained(false)
	{
		prev_->chained = true;
	}

	template<typename othermsg, typename otherfunc>
	TemplateDispatcher<TemplateDispatcher, othermsg, otherfunc> handle(otherfunc&& f_)
	{
		return TemplateDispatcher<TemplateDispatcher, othermsg, otherfunc>(q, this, std::forward<otherfunc>(f_));
	}

	~TemplateDispatcher() noexcept(false)
	{
		if (!chained)
			wait_and_dispatch();
	}
};

class Receiver
{
	Queue q;
public:
	operator Sender() { return Sender(&q); }
	Dispatcher wait()
	{
		return Dispatcher(&q);
	}
};
