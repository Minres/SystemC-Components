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
import com.minres.scviewer.database.ITrAttribute

class Attribute  implements ITrAttribute{
	
	AttrType attributeType

	def value
	
	Attribute(String name, DataType dataType, AssociationType type, value){
		attributeType = AttrTypeFactory.instance.getAttrType(name, dataType, type)
		switch(dataType){
			case DataType.STRING:
			case DataType.ENUMERATION:
				this.value=value[1..-2]
				break;
			default:
				this.value=value
		}
	}

	Attribute(AttrType other){
		attributeType=other
	}
	
	Attribute(AttrType other, value){
		this(other.name, other.dataType, other.type, value)
	}

	@Override
	public String getName() {
		return attributeType.getName();
	}

	@Override
	public AssociationType getType() {
		attributeType.type;
	}

	@Override
	public DataType getDataType() {
		attributeType.dataType;
	}

}
