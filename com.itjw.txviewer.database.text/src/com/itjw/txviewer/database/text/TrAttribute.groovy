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
