/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database;

import java.util.Collection;
import java.util.List;

public interface ITx extends Comparable<ITx>{

	public Long getId();
	
	public ITxStream<ITxEvent> getStream();
	
	public ITxGenerator getGenerator();
	
	public Long getBeginTime();
	
	public Long getEndTime();
	
	public int getConcurrencyIndex();
	
	public List<ITxAttribute> getAttributes();
	
	public Collection<ITxRelation> getIncomingRelations();

	public Collection<ITxRelation> getOutgoingRelations();
}
