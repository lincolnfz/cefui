#ifndef _combinthreadcomit_h_
#define _combinthreadcomit_h_
#pragma once
#include <windows.h>
#include <deque>
#include <mutex>
#include <memory>
#include <vector>
#include "IPC.h"
#include "include/cef_base.h"
#include "include/cef_browser.h"

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
			TYPE_WSTRING,
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

		void SetStringVal(const std::string& val){
			type_ = TYPE_STRING;
			strVal_ = val;
		}

		const std::string& GetStrVal() const{
			return strVal_;
		}

		void SetWStringVal(const std::wstring& val){
			type_ = TYPE_WSTRING;
			strwVal_ = val;
		}

		const std::wstring& GetWStrVal() const{
			return strwVal_;
		}
	protected:
		Type type_;
		bool boolVal_;
		int intVal_;
		double doubleVal_;
		std::wstring strwVal_;
		std::string strVal_;
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

		void AppendVal(const std::string& val){
			cyjh_value item;
			item.SetStringVal(val);
			list_.push_back(item);
		}

		void AppendVal(const std::wstring& val){
			cyjh_value item;
			item.SetWStringVal(val);
			list_.push_back(item);
		}

		const cyjh_value::Type& GetType(const unsigned int& idx) const{
			return list_[idx].GetType();
		}

		const bool& GetBooleanVal(const unsigned int& idx) const{
			return list_[idx].GetBooleanVal();
		}

		const int& GetIntVal(const unsigned int& idx) const{
			return list_[idx].GetIntVal();
		}

		const double& GetDoubleVal(const unsigned int& idx) const{
			return list_[idx].GetDoubleVal();
		}

		const std::string& GetStrVal(const unsigned int& idx) const{
			return list_[idx].GetStrVal();
		}

		const std::wstring& GetWStrVal(const unsigned int& idx) const{
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
		INSTRUCT_NULL,
	};

	class CombinThreadComit;

	class Instruct
	{
	public:
		friend CombinThreadComit;
		Instruct();
		virtual ~Instruct();

		void setName(const char* name){
			name_ = name;
		}

		const std::string& getName(){
			return name_;
		}

		cyjh_value_list& getList(){
			return list_;
		}

		void setBrowserID(const int& id){
			browserID_ = id;
		}

		const int& getBrowserID() const{
			return browserID_;
		}

		const int& getID() const{
			return id_;
		}

		void setSucc(const bool& succ){
			succ_ = succ;
		}

		const bool& getSucc() const{
			return succ_;
		}

		static void SerializationInstruct(const Instruct* inst, Pickle& pick);

		static bool ObjectInstruct(const Pickle& pick, Instruct* inst);

	protected:
		void setInstructType(InstructType type){
			type_ = type;
		}

		const InstructType& getInstructType() const{
			return type_;
		}

		void setID(const int& id){
			id_ = id;
		}


	private:
		InstructType type_;
		std::string name_;		
		int id_;
		int browserID_;
		bool succ_;
		cyjh_value_list list_;
	};

	typedef HANDLE BlockEvents[2];
	struct RequestContext 
	{
		int id_;
		BlockEvents events_;
		std::shared_ptr<Instruct> parm_; //收到的请求参数放在这里
		std::shared_ptr<Instruct> outval_; //收到的最终返回结果放在这里

		RequestContext(){			
			id_ = 0;
			events_[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
			events_[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		~RequestContext(){
			CloseHandle(events_[0]);
			CloseHandle(events_[1]);
		}
	};
	
	class CombinThreadComit : public CefBase
	{
	public:
		CombinThreadComit(ThreadType type);
		virtual ~CombinThreadComit();

		virtual void Request(CefRefPtr<CefBrowser>, Instruct& parm, std::shared_ptr<Instruct>& val) = 0;
		virtual void RecvData(const unsigned char*, DWORD);
	protected:
		void SendRequest(IPCUnit*, Instruct& parm, std::shared_ptr<Instruct>& val);
		void Response(IPCUnit* ipc, std::shared_ptr<Instruct>, const int& req_id);
		virtual void procRecvRequest(const std::shared_ptr<Instruct>);
		
		void pushRequestEvent(std::shared_ptr<RequestContext>&);

		void popRequestEvent();

		void pushRecvRequestID(int id);

		bool popRecvRequestID(int id);

		int generateID();

		//是不是和自已目前处理的请求id一样，如果自已有请求id,则忽略
		bool isSameMyReqID(int id);

	protected:
		std::shared_ptr<RequestContext> getReqStackTop(int id);

		ThreadType threadType_;
		std::deque<std::shared_ptr<RequestContext>> eventRequestStack_;
		std::mutex eventRequestStackMutex_;

		std::deque<int> eventResponsStack_;
		std::mutex eventResponseStackMutex_;

		std::mutex generateIDMutex_;

		int requestID_;

		static DWORD s_tid_;

		IMPLEMENT_REFCOUNTING(CombinThreadComit);
	};

}
#endif
