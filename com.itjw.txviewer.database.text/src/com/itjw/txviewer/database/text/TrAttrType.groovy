package com.itjw.txviewer.database.text

import com.itjw.txviewer.database.ITrAttrType;

class TrAttrType implements ITrAttrType {
	String name
	String type
	
	static TrAttrType getAttrType(String name, String type){
		TrAttrTypeFactory.instance.getAttrType(name, type)
	}
	
	TrAttrType(String name, String type){
		this.name=name
		this.type=type
	}
}
