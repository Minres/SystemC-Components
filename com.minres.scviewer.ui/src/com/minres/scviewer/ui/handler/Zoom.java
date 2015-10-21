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
package com.minres.scviewer.ui.handler;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.handlers.HandlerUtil;

import com.minres.scviewer.ui.TxEditorPart;

public class Zoom extends AbstractHandler {
	private static final String PARM_MSG = "com.minres.scviewer.ui.zoom.level";

	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException {
		IEditorPart editor = HandlerUtil.getActiveEditor(event);
	    String msg = event.getParameter(PARM_MSG);
	    if (msg == null) {
			if(editor instanceof TxEditorPart){
				((TxEditorPart)editor).setZoomFit();
			}
	    } else {
	    	Integer level = Integer.parseInt(msg);
			if(editor instanceof TxEditorPart){
				((TxEditorPart)editor).setZoomLevel(level);
			}
	    }
		return null;
	}

}
