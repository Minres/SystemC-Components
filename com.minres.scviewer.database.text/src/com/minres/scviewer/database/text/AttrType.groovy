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
import com.minres.scviewer.database.DataType;
import com.minres.scviewer.database.ITrAttrType;

class AttrType implements ITrAttrType {
	String name
	DataType dataType
	AssociationType type
	
	static AttrType getAttrType(String name, DataType dataType, AssociationType type){
		AttrTypeFactory.instance.getAttrType(name, dataType, type)
	}
	
	AttrType(String name, DataType dataType, AssociationType type){
		this.name=name
		this.dataType=dataType
		this.type=type
	}
}
