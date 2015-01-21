/*******************************************************************************
 * Copyright (c) 2014, 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database.sqlite;

import java.util.HashMap;

import com.minres.scviewer.database.RelationType;

class RelationTypeFactory {
	
	HashMap<String, RelationType> registry=new HashMap<String, RelationType>();
	
	public RelationType getRelationType(String name) {
		if(registry.containsKey(name)) return registry.get(name);
		RelationType rt = new RelationType(name);
		registry.put(name, rt);
		return rt;
	}

}
