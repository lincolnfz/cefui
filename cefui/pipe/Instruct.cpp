#include "stdafx.h"
#include "Instruct.h"

namespace cyjh{

	Instruct::Instruct()
	{
		type_ = Instruct::NONE_NIL;
		id_ = 0;
		succ_ = false;
		sender_ = 0;
		target_ = 0;
		fd_ = 0;
	}

	Instruct::~Instruct()
	{

	}

	void Instruct::SerializationInstruct(const Instruct* inst, Pickle& pick)
	{
		pick.WriteInt(inst->type_);
		pick.WriteInt(inst->id_);
		pick.WriteInt(inst->sender_);
		pick.WriteBool(inst->succ_);
		pick.WriteString(inst->name_);		
		int len = inst->list_.GetSize();
		pick.WriteInt(len);
		for (int idx = 0; idx < len; ++idx)
		{
			const cyjh_value::Type type = inst->list_.GetType(idx);
			pick.WriteInt(type);
			switch (type)
			{
			case cyjh_value::TYPE_BOOLEAN:
				{
					const bool val = inst->list_.GetBooleanVal(idx);
					pick.WriteBool(val);
				}
				break;
			case cyjh_value::TYPE_INTEGER:
				{
					const int val = inst->list_.GetIntVal(idx);
					pick.WriteInt(val);
				}
				break;
			case cyjh_value::TYPE_DOUBLE:
				{
					const double val = inst->list_.GetDoubleVal(idx);
					pick.WriteDouble(val);
				}
				break;
			case cyjh_value::TYPE_STRING:
				{
					const std::string val = inst->list_.GetStrVal(idx);
					pick.WriteString(val);
				}
				break;
			case cyjh_value::TYPE_WSTRING:
				{
					const std::wstring val = inst->list_.GetWStrVal(idx);
					pick.WriteWString(val);
				}
				break;
			default:
				break;
			}
		}
	}

	bool Instruct::ObjectInstruct(const Pickle& pick, Instruct* inst)
	{
		bool bret = true;
		cyjh::PickleIterator itor(pick);
		while (true){

			int type = 0;
			if(!pick.ReadInt(&itor, &type)){
				bret = false;
				break;
			}
			inst->setType((Instruct::_type)type);

			int id = 0;
			if(!pick.ReadInt(&itor, &id)){
				bret = false;
				break;
			}
			inst->setID(id);

			int sender = 0;
			if(!pick.ReadInt(&itor, &sender)){
				bret = false;
				break;
			}
			inst->setSender(sender);

			bool succ = false;
			if ( !pick.ReadBool(&itor, &succ) ){
				bret = false;
				break;
			}
			inst->setSucc(succ);

			std::string name;
			if ( !pick.ReadString(&itor, &name) ){
				bret = false;
				break;
			}
			inst->setName(name.c_str());

			int len = 0;
			pick.ReadInt(&itor, &len);
			for (int idx = 0; idx < len; ++idx)
			{
				int val = 0;
				pick.ReadInt(&itor, &val);
				cyjh_value::Type val_type = static_cast<cyjh_value::Type>(val);
				switch (val_type)
				{
				case cyjh::cyjh_value::TYPE_NULL:
					break;
				case cyjh::cyjh_value::TYPE_BOOLEAN:
					{
						bool val = false;
						if ( !pick.ReadBool(&itor, &val) ){
							bret = false;
						}
						inst->getList().AppendVal(val);
					}
					break;
				case cyjh::cyjh_value::TYPE_INTEGER:
					{
						int val = 0;
						if (!pick.ReadInt(&itor, &val)){
							bret = false;
						}
						inst->getList().AppendVal(val);
					}
					break;
				case cyjh::cyjh_value::TYPE_DOUBLE:
					{
						double val = 0.0;
						if (!pick.ReadDouble(&itor, &val)){
							bret = false;
						}
						inst->getList().AppendVal(val);
					}
					break;
				case cyjh::cyjh_value::TYPE_STRING:
					{
						std::string val;
						if (!pick.ReadString(&itor, &val)){
							bret = false;
						}
						inst->getList().AppendVal(val);
					}
					break;
				case cyjh::cyjh_value::TYPE_WSTRING:
					{
						std::wstring val;
						if (!pick.ReadWString(&itor, &val)){
							bret = false;
						}
						inst->getList().AppendVal(val);
					}
					break;
				case cyjh::cyjh_value::TYPE_BINARY:
					break;
				case cyjh::cyjh_value::TYPE_DICTIONARY:
					break;
				case cyjh::cyjh_value::TYPE_LIST:
					break;
				default:
					break;
				}

				if ( bret == false )
				{
					break;
				}
			}
			break;
		}

		return bret;
	}

}