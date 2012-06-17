package com.itjw.txviewer.database.text

import com.itjw.txviewer.database.ITrAttrType
import com.itjw.txviewer.database.ITrAttribute

class TrAttrTypeFactory {
	private static final instance = new TrAttrTypeFactory()
	
	def attributes = [:]
	
	private TrAttrTypeFactory() {
		TrAttrTypeFactory.metaClass.constructor = {-> instance }
	}
	
	ITrAttrType getAttrType(String name, String type){
		def key = name+":"+type
		ITrAttrType res
		if(attributes.containsKey(key)){
			res=attributes[key]
		} else {
			res=new TrAttrType(name, type)
			attributes[key]=res
		}
		return res
	}
}
