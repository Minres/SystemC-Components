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

import com.itjw.txviewer.database.ITrAttrType;

class TrAttrType implements ITrAttrType {
	String name
	String dataType
	ITrAttrType.AttributeType type
	
	static TrAttrType getAttrType(String name, String dataType, ITrAttrType.AttributeType type){
		TrAttrTypeFactory.instance.getAttrType(name, dataType, type)
	}
	
	TrAttrType(String name, String dataType, ITrAttrType.AttributeType type){
		this.name=name
		this.dataType=dataType
		this.type=type
	}
}
