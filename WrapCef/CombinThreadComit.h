#ifndef _combinthreadcomit_h_
#define _combinthreadcomit_h_
#pragma once
#include <windows.h>
#include <deque>
#include <mutex>
#include <memory>
#include <vector>
#include "IPC.h"

namespace cyjh{

	class cyjh_value
	{
	public:
		enum Type {
			TYPE_NULL = 0,
			TYPE_BOOLEAN,
			TYPE_INTEGER,
			TYPE_DOUBLE,
			TYPE_STRING,
			TYPE_BINARY,
			TYPE_DICTIONARY,
			TYPE_LIST
			// Note: Do not add more types. See the file-level comment above for why.
		};
		cyjh_value() :type_(TYPE_NULL){
			boolVal_ = false;
			intVal_ = 0;
			doubleVal_ = 0.0;
		}
		virtual ~cyjh_value(){}

		const Type& GetType() const{
			return type_;
		}

		void SetBoolean(const bool& val){
			type_ = TYPE_BOOLEAN;
			boolVal_ = val;
		}

		const bool& GetBooleanVal() const {
			return boolVal_;
		}

		void SetIntVal(const int& val){
			type_ = TYPE_INTEGER;
			intVal_ = val;
		}

		const int& GetIntVal() const {
			return intVal_;
		}

		void SetDoubleVal(const double& val){
			type_ = TYPE_DOUBLE;
			doubleVal_ = val;
		}

		const double& GetDoubleVal() const{
			return doubleVal_;
		}

		void SetWStringVal(const std::wstring& val){
			type_ = TYPE_STRING;
			strVal_ = val;
		}

		const std::wstring& GetWStrVal() const{
			return strVal_;
		}
	protected:
		Type type_;
		bool boolVal_;
		int intVal_;
		double doubleVal_;
		std::wstring strVal_;
	private:

	};

	class cyjh_value_list
	{
	public:
		typedef std::vector<cyjh_value>::iterator iterator;
		typedef std::vector<cyjh_value>::const_iterator const_iterator;
		cyjh_value_list(){}
		virtual ~cyjh_value_list(){}

		const size_t GetSize() const{
			return list_.size();
		}

		void AppendVal(const bool& val){
			cyjh_value item;
			item.SetBoolean(val);
			list_.push_back(item);
		}

		void AppendVal(const int& val){
			cyjh_value item;
			item.SetIntVal(val);
			list_.push_back(item);
		}

		void AppendVal(const double& val){
			cyjh_value item;
			item.SetDoubleVal(val);
			list_.push_back(item);
		}

		void AppendVal(const std::wstring& val){
			cyjh_value item;
			item.SetWStringVal(val);
			list_.push_back(item);
		}

		const cyjh_value::Type& GetType(const unsigned int& idx){
			return list_[idx].GetType();
		}

		const bool& GetBooleanVal(const unsigned int& idx){
			return list_[idx].GetBooleanVal();
		}

		const int& GetIntVal(const unsigned int& idx){
			return list_[idx].GetIntVal();
		}

		const double& GetDoubleVal(const unsigned int& idx){
			return list_[idx].GetDoubleVal();
		}

		const std::wstring& GetWStrVal(const unsigned int& idx){
			return list_[idx].GetWStrVal();
		}

		// Iteration.
		iterator begin() { return list_.begin(); }
		iterator end() { return list_.end(); }

		const_iterator begin() const { return list_.begin(); }
		const_iterator end() const { return list_.end(); }
	protected:
		std::vector<cyjh_value> list_;
	private:
	};

	enum ThreadType
	{
		THREAD_UI,
		THREAD_RENDER,
	};

	enum InstructType
	{
		INSTRUCT_REQUEST,
		INSTRUCT_RESPONSE,
	};

	class Instruct
	{
	public:
		Instruct();
		virtual ~Instruct();

		void setInstructType(InstructType type){
			type_ = type;
		}

		const InstructType& getInstructType() const{
			return type_;
		}

		void setName(const char* name){
			name_ = name;
		}

		const std::string& getName(){
			return name_;
		}

		cyjh_value_list& getList(){
			return list_;
		}

	private:
		InstructType type_;
		std::string name_;
		cyjh_value_list list_;
	};

	typedef HANDLE BlockEvents[2];
	struct RequestContext 
	{
		BlockEvents events_;
		std::shared_ptr<Instruct> parm_; //收到的请求参数放在这里
		std::shared_ptr<Instruct> outval_; //收到的最终返回结果放在这里

		RequestContext(){			
			events_[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
			events_[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		~RequestContext(){
			CloseHandle(events_[0]);
			CloseHandle(events_[1]);
		}
	};
	
	class CombinThreadComit
	{
	public:
		CombinThreadComit(ThreadType type);
		virtual ~CombinThreadComit();

		void Request(Instruct& parm, std::shared_ptr<Instruct> val);

		void Response(Instruct&);

	protected:
		virtual void procRecvRequest(std::shared_ptr<Instruct>);
		void RecvData(const byte*, DWORD);

	private:
		void pushEvent(std::shared_ptr<RequestContext>&);

		void popEvent();

		ThreadType threadType_;
		std::deque<std::shared_ptr<RequestContext>> eventStack_;
		std::mutex eventStackMutex_;
	};

}
#endif
