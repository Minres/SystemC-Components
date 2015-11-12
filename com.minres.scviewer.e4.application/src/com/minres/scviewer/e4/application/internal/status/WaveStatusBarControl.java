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
package com.minres.scviewer.e4.application.internal.status;

import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.di.UISynchronize;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.jface.action.ContributionItem;
import org.eclipse.jface.action.StatusLineManager;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CLabel;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;

public class WaveStatusBarControl extends StatusBarControl {

	public static final String ZOOM_LEVEL="ZoomLevelUpdate";
	public static final String CURSOR_TIME="CursorPosUpdate";
	public static final String MARKER_TIME="MarkerPosUpdate";
	public static final String MARKER_DIFF="MarlerDiffUpdate";

	@Inject
	EModelService modelService;

	class TextContributionItem extends ContributionItem {

		final String labelString;
		final int width;
		CLabel label, text;
		private String content;

		public TextContributionItem(String labelString, int width) {
			super();
			this.labelString = labelString;
			this.width=width;
			content="";
		}

		@Override
		public void fill(Composite parent) {
			Composite box=new Composite(parent, SWT.NONE);
			box.setLayout(new GridLayout(2, false));
			label=new CLabel(box, SWT.SHADOW_NONE);	
			label.setText(labelString);
			text=new CLabel(box, SWT.SHADOW_IN);
			GridData layoutData=new GridData(SWT.DEFAULT, SWT.DEFAULT, true, false);
			layoutData.minimumWidth=width;
			text.setLayoutData(layoutData);
		}

		@Override
		public boolean isDynamic() {
			return true;
		}

		public void setText(String message){
			this.content=message;
			if(text!=null && !text.isDisposed()) text.setText(content);
		}

	}

	TextContributionItem cursorContribution, markerContribution, markerDiffContribution, zoomContribution;

	@Inject
	public WaveStatusBarControl(UISynchronize sync) {
		super(sync);
		cursorContribution = new TextContributionItem("C:", 80);
		markerContribution = new TextContributionItem("M:", 80);
		markerDiffContribution = new TextContributionItem("C-M:", 80);
		zoomContribution = new TextContributionItem("Z:", 80);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP,cursorContribution);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP,markerContribution);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP,markerDiffContribution);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP, zoomContribution);
	}

	@Inject
	public void setSelection(@Named(IServiceConstants.ACTIVE_SELECTION)@Optional IStructuredSelection selection){
		if(manager!=null && selection!=null){
			switch(selection.size()){
			case 0:
				manager.setMessage("");
				break;
			case 1:
				manager.setMessage(selection.getFirstElement().getClass().getSimpleName()+" selected");
				break;
			default:
				manager.setMessage(""+selection.size()+" Elements");
				break;
			}
		}
	}

	@Inject @Optional
	public void  getZoomEvent(@UIEventTopic(ZOOM_LEVEL) String text) {
		zoomContribution.setText(text);
	} 

	@Inject @Optional
	public void  getCursorEvent(@UIEventTopic(CURSOR_TIME) String text) {
		cursorContribution.setText(text);
	} 

	@Inject @Optional
	public void  getMarkerEvent(@UIEventTopic(MARKER_TIME) String text) {
		markerContribution.setText(text);
	} 

	@Inject @Optional
	public void  getDiffEvent(@UIEventTopic(MARKER_DIFF) String text) {
		markerDiffContribution.setText(text);
	} 

}