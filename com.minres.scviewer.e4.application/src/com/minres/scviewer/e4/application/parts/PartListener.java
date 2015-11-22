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
package com.minres.scviewer.e4.application.parts;

import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.workbench.modeling.IPartListener;

/**
 * The default implementation of a {@link IPartListener}.
 * The class that is interested in processing a part
 * event extends this class overriding the respective method, and the object created
 * with that class is registered with a component using the
 * component's <code>addPartListener<code> method. When
 * the part event occurs, that object's appropriate
 * method is invoked.
 *
 * @see PartEvent
 */
public class PartListener implements IPartListener {
	
	/* (non-Javadoc)
	 * @see org.eclipse.e4.ui.workbench.modeling.IPartListener#partBroughtToTop(org.eclipse.e4.ui.model.application.ui.basic.MPart)
	 */
	@Override
	public void partBroughtToTop(MPart part) {}

	/* (non-Javadoc)
	 * @see org.eclipse.e4.ui.workbench.modeling.IPartListener#partActivated(org.eclipse.e4.ui.model.application.ui.basic.MPart)
	 */
	@Override
	public void partActivated(MPart part) {}

	/* (non-Javadoc)
	 * @see org.eclipse.e4.ui.workbench.modeling.IPartListener#partDeactivated(org.eclipse.e4.ui.model.application.ui.basic.MPart)
	 */
	@Override
	public void partDeactivated(MPart part) {}

	/* (non-Javadoc)
	 * @see org.eclipse.e4.ui.workbench.modeling.IPartListener#partHidden(org.eclipse.e4.ui.model.application.ui.basic.MPart)
	 */
	@Override
	public void partHidden(MPart part) {}

	/* (non-Javadoc)
	 * @see org.eclipse.e4.ui.workbench.modeling.IPartListener#partVisible(org.eclipse.e4.ui.model.application.ui.basic.MPart)
	 */
	@Override
	public void partVisible(MPart part) {}
}