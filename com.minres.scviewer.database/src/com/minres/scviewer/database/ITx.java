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
package com.minres.scviewer.database;

import java.util.Collection;
import java.util.List;

public interface ITx extends Comparable<ITx>{

	public Long getId();
	
	public ITxStream getStream();
	
	public ITxGenerator getGenerator();
	
	public EventTime getBeginTime();
	
	public EventTime getEndTime();
	
	public List<ITxAttribute> getAttributes();
	
	public Collection<ITxRelation> getIncomingRelations();

	public Collection<ITxRelation> getOutgoingRelations();
}
