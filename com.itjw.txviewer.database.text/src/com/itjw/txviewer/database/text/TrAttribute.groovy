package com.itjw.txviewer.database.text

import com.itjw.txviewer.database.ITrAttribute

class TrAttribute  implements ITrAttribute{
	
	TrAttrType attributeType
	def value
	
	TrAttribute(String name, String type, value){
		attributeType = TrAttrTypeFactory.instance.getAttrType(name, type)
		switch(type){
			case "STRING":
			case "ENUMERATION":
				this.value=value[1..-2]
				break;
			default:
				this.value=value
		}
	}

	TrAttribute(TrAttrType other){
		attributeType=other
	}
	
	TrAttribute(TrAttrType other, value){
		this(other.name, other.type, value)
	}

	@Override
	public String getName() {
		return attributeType.getName();
	}

	@Override
	public String getType() {
		return attributeType.getType();
	}

}
