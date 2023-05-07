#pragma once

class Base
{
	public:
		Base(const int val) : val_(val)
		{}

		virtual ~Base() = default;

		virtual int getVal() const
		{
			return val_;
		}

	private:
		int val_{0};
};

class Child : public Base
{
	public:
		Child(const int val) : Base(val)
		{}

		int getVal() const
    {
      return val_;
    }
};
