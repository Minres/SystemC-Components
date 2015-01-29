package com.minres.scviewer.ui.handler;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.handlers.HandlerUtil;
import org.eclipse.ui.internal.misc.StringMatcher;

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
