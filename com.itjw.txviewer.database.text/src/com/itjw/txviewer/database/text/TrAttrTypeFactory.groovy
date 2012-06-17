/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
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
