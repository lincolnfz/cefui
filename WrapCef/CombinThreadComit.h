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
#include "BlockThread.h"

#define OP_TIMEOUT 3000

#define PROC_STATE_NIL 0
#define PROC_STATE_FIN 1
#define PROC_STATE_REJ 1

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
		INSTRUCT_INQUEUE,
		INSTRUCT_OUTQUEUE,
		INSTRUCT_WAKEUP,
		INSTRUCT_REGBROWSER,
		INSTRUCT_NULL,
	};

	class CombinThreadComit;
	class UIThreadCombin;
	class RenderThreadCombin;
	struct MaybeLockItem;

	class Instruct
	{
	public:
		friend CombinThreadComit;
		friend UIThreadCombin;
		friend RenderThreadCombin;
		friend MaybeLockItem;
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

		const int& getAtom() const{
			return atom_;
		}

		const int& getProcState(){
			return procState_;
		}

		void setSucc(const bool& succ){
			succ_ = succ;
		}

		const bool& getSucc() const{
			return succ_;
		}

		static void SerializationInstruct(const Instruct* inst, Pickle& pick);

		static bool ObjectInstruct(const Pickle& pick, Instruct* inst);

		void setInstructType(InstructType type){
			type_ = type;
		}

	protected:		

		const InstructType& getInstructType() const{
			return type_;
		}

		void setID(const int& id){
			id_ = id;
		}

		void setAtom(const int& atom){
			atom_ = atom;
		}

		void setProcState(const int& procState){
			procState_ = procState;
		}

		void setNewSession(const bool& bNew){
			newSession_ = bNew;
		}

		void setProcTimeout(const bool& bTimeout){
			procTimeout_ = bTimeout;
		}

		const bool& newSession() const{
			return newSession_;
		}

		const bool& procTimeout() const{
			return procTimeout_;
		}

	private:
		InstructType type_;
		std::string name_;		
		int id_;
		int browserID_;
		int atom_;
		int procState_; //PROC_STATE_NUL,需要发送请求处理 PROC_STATE_FIN,处理完毕 PROC_STATE_REJ,对方繁忙,处理失败
		bool succ_;
		bool newSession_;
		bool procTimeout_;
		cyjh_value_list list_;
	};

	typedef HANDLE BlockEvents[2];
	struct RequestContext 
	{
		int id_;
		int atom_;
		bool bResponse;
		BlockEvents events_;
		std::shared_ptr<Instruct> parm_; //收到的请求参数放在这里
		std::shared_ptr<Instruct> outval_; //收到的最终返回结果放在这里

		RequestContext(){			
			id_ = 0;
			atom_ = 0;
			bResponse = true;
			events_[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
			events_[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		~RequestContext(){
			CloseHandle(events_[0]);
			CloseHandle(events_[1]);
		}
	};

	//记录收到的请求上下文id
	struct RecvReqItem
	{
		RecvReqItem(const int& id, const int& atom){
			id_ = id;
			atom_ = atom;
		}
		int id_;
		int atom_;
	};

	struct MaybeProcItem
	{
		MaybeProcItem(const int& id, const int& atom) :reqContext_(id, atom){
			hitProc_ = false;
		}
		RecvReqItem reqContext_;
		bool hitProc_;
	};

	struct ReqInfo
	{
	public:
		ReqInfo(){
			len_ = 0;
			pData_ = NULL;
		}
		void set(const unsigned char* data, int len){
			len_ = len;
			pData_ = new unsigned char[len];
			memcpy_s(pData_, len, data, len);
		}
		~ReqInfo(){
			delete pData_;
		}
		unsigned char* pData_;
		int len_;

	};

	class CombinThreadComit;

	class SyncRequestQueue : public CDataProcessQueue<ReqInfo>
	{
	public:
		SyncRequestQueue(CombinThreadComit* obj) :obj_(obj){
			hEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
		}

		virtual ~SyncRequestQueue(){
			CloseHandle(hEvent_);
		}

		virtual BOOL ProcDataPack(std::shared_ptr<ReqInfo> sp);

		BOOL SubmitPack(std::shared_ptr<ReqInfo> pack);

		void SetEvent(){
			::SetEvent(hEvent_);
		}

		void ResetEvent(){
			::ResetEvent(hEvent_);
		}

	private:
		CombinThreadComit* obj_;
		HANDLE hEvent_;
	};

	struct MaybeLockItem
	{
		std::shared_ptr<Instruct> spRemote_Req_;
		//HANDLE hEvent_;
		CombinThreadComit* srv_;
		//HANDLE hThread_;
		HANDLE hTimer_;
		bool bTimeout_;

		static unsigned int __stdcall WaitReqTimeOut(void * parm);

		static void __stdcall WaitOrTimerCallback(
			PVOID   lpParameter,
			BOOLEAN TimerOrWaitFired);

		void cancelTimer();
		MaybeLockItem(std::shared_ptr<Instruct>& remote_req, CombinThreadComit* srv);

		~MaybeLockItem();
	};

	class CombinThreadComit : public CefBase
	{
		friend UIBlockThread;
		friend RenderBlockThread;
		friend MaybeLockItem;
	public:
		CombinThreadComit(ThreadType type);
		virtual ~CombinThreadComit();

		virtual void Request(CefRefPtr<CefBrowser>, Instruct& parm, std::shared_ptr<Instruct>& val) = 0;
		virtual void RecvData(const unsigned char*, DWORD);
		void WakeUp();
		void SendRenderWakeUpHelp(int browserID/*, int reqid, int atom*/);

	protected:
		void SendRequest(IPCUnit*, Instruct& parm, std::shared_ptr<Instruct>& val);
		void Response(IPCUnit* ipc, std::shared_ptr<Instruct>, const int& req_id, const int& req_atom);
		virtual void procRecvRequest(const std::shared_ptr<Instruct>) = 0;
		virtual bool prepareResponse(const std::shared_ptr<Instruct> parm);
		bool RegisterReqID(IPCUnit* ipc, const int browser_id, const int req_id);
		void UnRegisterReqID(IPCUnit* ipc, int req_id);
		
		void pushRequestEvent(std::shared_ptr<RequestContext>&);

		bool popRequestEvent(int reqid);

		bool pushRecvRequestID(int id, int atom);

		bool popRecvRequestID(int id, int atom);

		bool matchRecvRequestID(int id);

		void pushPengingRequest(std::shared_ptr<Instruct> sp);

		std::shared_ptr<Instruct> getTopPengingRequest();

		bool removePendingReq(int id, int atom);

		bool isEmptyPengingReq();

		void checkPendingReq();

		//这个在请求函数处理完执行，防止请求函数处理时，收到新的请求，因为respone中只能处理一个id,而放在pengding队列中
		void proxy_checkPendingReq();

		static unsigned int __stdcall ProcForkReq(void * parm);

		void pushMaybelockQueue(std::shared_ptr<Instruct>& spReq);

		void removeMaybelockQueue(const std::shared_ptr<Instruct>& spReq);

		bool checkMaybelockQueue();

		void pushProcedQueue(int id, int atom);

		bool hitProcedQueue(int id, int atom);

		bool removeProcedQueue(int id, int atom);

		void checkMaybeLostInstruct(std::shared_ptr<Instruct>& spReq);

		bool isRecvRequestEmpty();

		bool isSendRenderImmedNoBlockEmpty();

		bool isSendRegBrowserEmpty();

		int generateID();

		//是不是和自已目前处理的请求id一样，如果自已有请求id,则忽略
		bool isSameMyReqID(int id);

		//注册浏览器不放在公共调用模块中执行
		void RegisertBrowserHelp(std::shared_ptr<Instruct> spInfo);

		//拒绝请求,当已在处理一个会话时,不会接受新的会话
		virtual void RejectReq(std::shared_ptr<Instruct> spInfo) = 0;

		virtual void ProcTrunkReq(std::shared_ptr<Instruct> spInfo);

		void ProcRecvDataHelp(std::shared_ptr<Instruct> spInfo);

	protected:
		std::shared_ptr<RequestContext> getReqStackNearlTopID(int id);
		std::shared_ptr<RequestContext> getReqStackTop();

		ThreadType threadType_;
		std::deque<std::shared_ptr<RequestContext>> eventRequestStack_;
		std::mutex eventRequestStackMutex_;

		std::deque<RecvReqItem> eventResponsStack_;
		std::mutex eventResponseStackMutex_;

		std::mutex generateIDMutex_;

		std::mutex newSessinBlockMutex_;

		std::deque<int> sendRenderImmedNoBlockQueue_;
		std::mutex sendRenderImmedNoBlockQueue_Mutex_;

		std::deque<int> sendRegBrowserQueue_;
		std::mutex sendRegBrowserQueue_Mutex_;

		std::deque<std::shared_ptr<Instruct>> pendingProcReqQueue_;
		std::mutex pendingProcReqQueue_Mutex_;

		std::deque<std::shared_ptr<MaybeLockItem>> maybeLockReqQueue_;
		std::mutex maybeLockReqQueue_Mutex_;

		std::deque<MaybeProcItem> hasProcedQueue_;
		std::mutex hasProcedQueue_Mutex_;

		int requestID_;

		static DWORD s_tid_;

		//SyncRequestQueue requestQueue_;

		//std::deque<std::shared_ptr<ReqInfo>> pendingReqQueue_;
		BlockThread* blockThread_;

		HANDLE m_hTimeQueue;

		IMPLEMENT_REFCOUNTING(CombinThreadComit);
	};

}
#endif
