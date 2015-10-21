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
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.handlers.HandlerUtil;

import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.ui.TxEditorPart;

public class RemoveHandler extends AbstractHandler {

	@SuppressWarnings("unchecked")
	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException {
		IEditorPart editor = HandlerUtil.getActiveEditor(event);
		if(editor instanceof TxEditorPart){
			TxEditorPart editorPart = (TxEditorPart) editor;
			ISelection selection =editorPart.getSelection();
			if(selection instanceof StructuredSelection && ((StructuredSelection)selection).getFirstElement() instanceof IWaveform<?>){
				editorPart.removeStreamFromList((IWaveform<? extends IWaveformEvent>) ((StructuredSelection)selection).getFirstElement());
				editorPart.setSelection(new StructuredSelection());
			}
		}
		return null;
	}

}
