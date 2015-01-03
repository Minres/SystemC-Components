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
package com.minres.scviewer.database.text

import com.minres.scviewer.database.AssociationType;
import com.minres.scviewer.database.DataType
import com.minres.scviewer.database.ITrAttrType
import com.minres.scviewer.database.ITrAttribute

class AttrTypeFactory {
	private static final instance = new AttrTypeFactory()
	
	def attributes = [:]
	
	private AttrTypeFactory() {
		AttrTypeFactory.metaClass.constructor = {-> instance }
	}
	
	ITrAttrType getAttrType(String name, DataType dataType, AssociationType type){
		def key = name+":"+dataType.toString()
		ITrAttrType res
		if(attributes.containsKey(key)){
			res=attributes[key]
		} else {
			res=new AttrType(name, dataType, type)
			attributes[key]=res
		}
		return res
	}
}
