#ifndef Instruct_h
#define Instruct_h
#include <vector>
#include "cjpickle.h"

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

		void AppendVal(const char* val){
			cyjh_value item;
			item.SetStringVal(std::string(val));
			list_.push_back(item);
		}

		void AppendVal(const wchar_t* val){
			cyjh_value item;
			item.SetWStringVal(std::wstring(val));
			list_.push_back(item);
		}

		const cyjh_value::Type& GetType(const unsigned int& idx) const{
			if ( idx + 1 > list_.size() )
			{
				static cyjh_value::Type val = cyjh_value::TYPE_NULL;
				return val;
			}
			return list_[idx].GetType();
		}

		const bool& GetBooleanVal(const unsigned int& idx) const{
			if ( idx + 1 > list_.size() )
			{
				static bool val = false;
				return val;
			}
			return list_[idx].GetBooleanVal();
		}

		const int& GetIntVal(const unsigned int& idx) const{
			if ( idx + 1 > list_.size() )
			{
				static int val = 0;
				return val;
			}
			return list_[idx].GetIntVal();
		}

		const double& GetDoubleVal(const unsigned int& idx) const{
			if ( idx + 1 > list_.size() )
			{
				static double val = 0.0f;
				return val;
			}
			return list_[idx].GetDoubleVal();
		}

		const std::string& GetStrVal(const unsigned int& idx) const{
			if ( idx + 1 > list_.size() )
			{
				static std::string val;
				return val;
			}
			return list_[idx].GetStrVal();
		}

		const std::wstring& GetWStrVal(const unsigned int& idx) const{
			if ( idx + 1 > list_.size() )
			{
				static std::wstring val;
				return val;
			}
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

	class Instruct
	{
	public:
		enum _type{
			NONE_NIL,
			RECV_REQ,
			RECV_ACK,
		};
		Instruct();
		virtual ~Instruct();

		void setID(const int& id){
			id_ = id;
		}

		const int& getID(){
			return id_;
		}

		void setFD(const int& fd){
			fd_ = fd;
		}

		const int& getFD(){
			return fd_;
		}

		void setSender(const int& sender){
			sender_ = sender;
		}

		const int& getSender(){
			return sender_;
		}

		void setTarget(const int& target){
			target_ = target;
		}

		const int& getTarget(){
			return target_;
		}

		void setType(const _type& type){
			type_ = type;
		}

		const _type& getType(){
			return type_;
		}
		
		void setName(const char* name){
			name_ = name;
		}
		const std::string& getName(){
			return name_;
		}

		void setSucc(const bool& succ){
			succ_ = succ;
		}

		const bool& getSucc() const{
			return succ_;
		}

		cyjh_value_list& getList(){
			return list_;
		}
		static void SerializationInstruct(const Instruct* inst, Pickle& pick);
		static bool ObjectInstruct(const Pickle& pick, Instruct* inst);
	private:
		_type type_;
		int id_;
		int fd_;
		bool succ_;
		int sender_;
		int target_;
		std::string name_;
		cyjh_value_list list_;		
	};
}

#endif