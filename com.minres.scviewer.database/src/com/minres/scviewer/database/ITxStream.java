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
import java.util.NavigableMap;

public interface  ITxStream<T extends ITxEvent> extends IWaveform<T> {

	public List<ITxGenerator> getGenerators();

	public int getMaxConcurrency();

	public NavigableMap<Long, Collection<T>> getEvents();

	public Collection<T> getWaveformEventsAtTime(Long time);

}
