(function (global, factory) {
    typeof exports === 'object' && typeof module !== 'undefined' ? factory(exports, require('d3'), require('elkjs')) :
    typeof define === 'function' && define.amd ? define(['exports', 'd3', 'elkjs'], factory) :
    (global = typeof globalThis !== 'undefined' ? globalThis : global || self, factory(global.d3 = global.d3 || {}, global.d3, global.ELK));
})(this, (function (exports, d3, ELK) { 'use strict';

    function _interopDefaultLegacy (e) { return e && typeof e === 'object' && 'default' in e ? e : { 'default': e }; }

    function _interopNamespace(e) {
        if (e && e.__esModule) return e;
        var n = Object.create(null);
        if (e) {
            Object.keys(e).forEach(function (k) {
                if (k !== 'default') {
                    var d = Object.getOwnPropertyDescriptor(e, k);
                    Object.defineProperty(n, k, d.get ? d : {
                        enumerable: true,
                        get: function () { return e[k]; }
                    });
                }
            });
        }
        n["default"] = e;
        return Object.freeze(n);
    }

    var d3__namespace = /*#__PURE__*/_interopNamespace(d3);
    var ELK__default = /*#__PURE__*/_interopDefaultLegacy(ELK);

    var PORT_MARKERS = {
    		"WEST": {
    			"INPUT": "#westInPortMarker",
    			"OUTPUT": "#westOutPortMarker"},
    		"EAST": {
    			"INPUT": "#eastInPortMarker",
    			"OUTPUT": "#eastOutPortMarker"},
    		"NORTH": {
    			"INPUT": "#northInPortMarker",
    			"OUTPUT": "#northOutPortMarker"},
    	    "SOUTH": {
    	    	"INPUT": "#southInPortMarker" ,
    	    	"OUTPUT": "#southOutPortMarker"},
    };

    function addMarkers(defs, PORT_PIN_SIZE) {
        // real size of marker
        var w = 7;
        var h = 10;
        
        function addMarker(id, arrowTranslate, arrowRotate=0) {
        	var rightArrow = "M 0 4  2 4  2 0  7 5  2 10  2 6  0 6 Z";
            var trans = "";
        
            if (arrowTranslate[0] !== 0 || arrowTranslate[1] !== 0)
            	trans += "translate(" + arrowTranslate[0] + ", " + arrowTranslate[1] + ")";
        
            if (arrowRotate !== 0)
            	trans += "rotate(" + arrowRotate + ")";
        
            var cont = defs.append("g");
        
            cont
            .attr("id", id)
            .attr("class", "port")
            .append("path")
            .attr("d", rightArrow);
            
            if (trans)
            	cont
                .attr("transform", trans);
        }
        
        var horizYOffset = (PORT_PIN_SIZE[1] - h) * 0.5;
        var horizYOffset2 = (PORT_PIN_SIZE[1] + h) * 0.5;
        
        var vertXOffset = -(PORT_PIN_SIZE[1] - w) * 0.5;
        addMarker("westInPortMarker", [0, horizYOffset]);
        addMarker("westOutPortMarker",[w, horizYOffset2], 180);
        
        addMarker("eastInPortMarker", [w, horizYOffset2], 180);
        addMarker("eastOutPortMarker",[0, horizYOffset]);
        
        addMarker("northInPortMarker", [vertXOffset, -w], 90);
        addMarker("northOutPortMarker",[vertXOffset, 0], 270);
        
        addMarker("southInPortMarker", [vertXOffset, w], 270);
        addMarker("southOutPortMarker",[vertXOffset, 0], 90);
    }

    function getIOMarker(d) {
        var side = d.properties.side;
        var portType = d.direction;
        var marker = PORT_MARKERS[side][portType];
        if (marker === undefined) {
        	throw new Error("Wrong side, portType", side, portType)
        }
        return marker;
    }

    function portLevel(port){
    	if(!port.parent) return 0;
    	else {
    		return portLevel(port.parent)+1;
    	}

    }

    /*
     * Basic renderer which renders node as a box with ports, optionally with the body text
     */
    class GenericNodeRenderer {
        /**
         * @param schematic instance of HwSchematic
         **/
    	constructor(schematic) {
    		this.schematic = schematic;
    	}
        /**
         * check if this selector should be used for this node
         **/
    	selector(node) {
    		// always return true, because this is a default renderer which just renders a box with ports
    		return true;
    	}

    	getNodeLabelWidth(d) {
    		var schematic = this.schematic;
    		var widthOfText = schematic.widthOfText.bind(schematic);
    		return widthOfText(d.hwMeta.name);
    	}

        /**
         * Init bodyText and resolve size of node from body text and ports
         *
         * @param d component node
         *
         */
    	initNodeSizes(d) {
    		var schematic = this.schematic;
    		if (d.properties["org.eclipse.elk.noLayout"])
    			return;
    		var widthOfText = schematic.widthOfText.bind(schematic);

    		var labelW = this.getNodeLabelWidth(d);
    		var max = Math.max;
    		var bodyTextSize = this.initBodyTextLines(d);
    		const MBT = schematic.MAX_NODE_BODY_TEXT_SIZE;
    		bodyTextSize[0] = Math.min(bodyTextSize[0], MBT[0]);
    		bodyTextSize[1] = Math.min(bodyTextSize[1], MBT[1]);

    		// {PortSide: (portCnt, portWidth)}
    		var portDim = {
    			"WEST": [0, 0],
    			"EAST": [0, 0],
    			"SOUTH": [0, 0],
    			"NORTH": [0, 0]
    		};
    		var PORT_PIN_SIZE_x = schematic.PORT_PIN_SIZE[0],
    			PORT_PIN_SIZE_y = schematic.PORT_PIN_SIZE[1];
    		var CHAR_WIDTH = schematic.CHAR_WIDTH;
    		if (d.ports != null)
    			d.ports.forEach(function(p) {
    				var t = p.properties.side;
    				var indent = 0;
    				if (portLevel(p) > 0)
    					indent = (portLevel(p)+1) * CHAR_WIDTH;
    				var portW = widthOfText(p.hwMeta.name) + indent;
    				var pDim = portDim[t];
    				if (pDim === undefined)
    					throw new Error(t);
    				pDim[0]++;
    				pDim[1] = max(pDim[1], portW);

    				// dimension of connection pin
    				p.width = PORT_PIN_SIZE_x;
    				p.height = PORT_PIN_SIZE_y;
    			});

    		var west = portDim["WEST"],
    			east = portDim["EAST"],
    			south = portDim["SOUTH"],
    			north = portDim["NORTH"];

    		var portColums = 0;
    		if (west[0] && west[1] > 0)
    			portColums += 1;
    		if (east[0] && east[1] > 0)
    			portColums += 1;

    		var middleSpacing = 0;
    		if (portColums == 2)
    			middleSpacing = schematic.NODE_MIDDLE_PORT_SPACING;
    		var portW = max(west[1], east[1]);

    		d.portLabelWidth = portW;
    		d.width = max(portW * portColums + middleSpacing, labelW,
    			max(south[0], north[0]) * schematic.PORT_HEIGHT)
    			+ bodyTextSize[0] + CHAR_WIDTH;
    		d.height = max(max(west[0], east[0]) * schematic.PORT_HEIGHT,
    			bodyTextSize[1],
    			max(south[1], north[1]) * CHAR_WIDTH);
    	}

        /**
         * Split bodyText of one to lines and resolve dimensions of body text
         *
         * @param d component node
         */
    	initBodyTextLines(d) {
    		var schematic = this.schematic;
    		var max = Math.max;
    		var bt = d.hwMeta.bodyText;
    		if (bt) {
    			if (typeof bt === "string") {
    				bt = d.hwMeta.bodyText = bt.split("\n");
    			}
    			var bodyTextW = 0;
    			bt.forEach(function(line) {
    				bodyTextW = max(bodyTextW, line.length);
    			});
    			bodyTextW *= schematic.CHAR_WIDTH;
    			var bodyTextH = bt.length * schematic.CHAR_HEIGHT;
    		} else {
    			var bodyTextW = 0;
    			var bodyTextH = 0;
    		}
    		var pad = schematic.BODY_TEXT_PADDING;
    		if (bodyTextW > 0)
    			bodyTextW += pad[1] + pad[3];
    		if (bodyTextH > 0)
    			bodyTextH += pad[0] + pad[2];
    		return [bodyTextW, bodyTextH];
    	}

        /**
         * @param bodyTexts list of strings
         */
    	renderTextLines(bodyTexts) {
    		var schematic = this.schematic;
    		const padTop = schematic.BODY_TEXT_PADDING[0];
    		const padLeft = schematic.BODY_TEXT_PADDING[3];
    		const MBT = schematic.MAX_NODE_BODY_TEXT_SIZE;
    		const CHAR_WIDTH = schematic.CHAR_WIDTH;
    		const CHAR_HEIGHT = schematic.CHAR_HEIGHT;

    		bodyTexts.each(function() {
    			var bodyText = d3__namespace.select(this);
    			var d = bodyText.data()[0];
    			var bodyTextLines = d.hwMeta.bodyText;
    			var _MBT = [MBT[0] / CHAR_WIDTH, MBT[1] / CHAR_HEIGHT];

    			if (bodyTextLines && (!d.children
    				|| d.children.length == 0)) {
    				bodyTextLines.forEach(function(line, dy) {
    					if (line.length > _MBT[0])
    						line = line.slice(0, _MBT[0] - 3) + "...";
    					if (dy > _MBT[1])
    						return;
    					bodyText
    						.append("tspan")
    						.attr("x", d.portLabelWidth + padLeft)
    						.attr("y", padTop)
    						.attr("dy", dy + "em")
    						.text(line);
    				});
    			}
    		});
    	}

        /**
         * Prepare node before ELK processing
         * */
    	prepare(node) {
    		this.initNodeSizes(node);
    	}

        /**
         * Render svg of node
         *
         * @param root root svg element where nodes should be rendered
         * @param nodeG svg g for each node with data binded
         * */
    	render(root, nodeG) {
    		var node = nodeG
    			.attr("class", function(d) {
    				var cssClass;
    				if (d.hwMeta && d.hwMeta.isExternalPort) {
    					cssClass = "node-external-port";
    				} else {
                        let depth = 0;
                        let parent = d.hwMeta.parent;
                        while (parent) {
                            ++depth;
                            parent = parent.hwMeta.parent;
                        }
                        if (depth % 2 === 0) {
                            cssClass = "node node-0";
                        } else {
                            cssClass = "node node-1";
                        }
    				}
    				if (d.hwMeta.cssClass) {
    					cssClass += " " + d.hwMeta.cssClass;
    				}
    				return cssClass;
    			})
    			.attr("style", function(d) { return d.hwMeta.cssStyle; });
    		var nodeBody = node.append("rect");
    		// set dimensions and style of node
    		nodeBody
    			.attr("width", function(d) { return d.width })
    			.attr("height", function(d) { return d.height })
    			.attr("rx", 5) // rounded corners
    			.attr("ry", 5);

    		// apply node positions
    		node
    			.attr("transform", function(d) {
    				if (typeof d.x === "undefined" || typeof d.x === "undefined") {
    					throw new Error("Node with undefined position", d);
    				}
    				return "translate(" + d.x + " " + d.y + ")"
    			});

    		// spot node label
    		node.append("text")
    			.text(function(d) {
    				if (d.hwMeta && !d.hwMeta.isExternalPort) {
    					return d.hwMeta.name;
    				} else {
    					return "";
    				}
    			});

    		// spot node body text
    		node.append("text")
    			.call(this.renderTextLines.bind(this));

    		this.renderPorts(node);
    	}

    	renderPorts(node) {
    		var schematic = this.schematic;
    		var PORT_HEIGHT = schematic.PORT_HEIGHT;
    		var CHAR_WIDTH = schematic.CHAR_WIDTH;
    		var portG = node.selectAll(".port")
    			.data(function(d) {
    				return d.ports || [];
    			})
    			.enter()
    			.append("g")
    			.attr("style", (d) => d.hwMeta.cssStyle)
    			.attr("class", (d) => {
    				if (d.hwMeta.cssStyle) {
    					return "port " + d.hwMeta.cssClass;
    				} else {
    					return "port";
    				}
    			});

    		// apply port positions
    		portG
    			.attr("transform", function(d) {
    				return "translate(" + d.x + "," + d.y + ")"
    			});

    		node.each(function(d) {
    			var ignorePortLabel = typeof d.children !== "undefined";
    			if (d.ports != null) {
    				d.ports.forEach(function(p) {
    					p.ignoreLabel = ignorePortLabel;
    				});
    			}
    		});

    		// spot port name
    		portG.append("text")
    			.text(function(d, i) {
                    /*var next_d = port_data[i+1];
                    if (next_d && next_d.hwMeta.level > d.hwMeta.level) {
    					console.log(d.hwMeta.name);
                        //d.hwMeta.name=toString("+");
                    }
                    */
    				if (d.ignoreLabel)
    					return "";
    				else if (d.parent) {
    					var indent = '-'.repeat(portLevel(d));
    					var side = d.properties.side;
    					if (side == "WEST") {
    						return indent + d.hwMeta.name;					} else if (side == "EAST") {
    						return d.hwMeta.name + indent;
    					} else {
    						throw new Error(side);
    					}
    				} else
    					return d.hwMeta.name;
    			})
    			.attr("x", function(d) {
    				var side = d.properties.side;
    				if (side == "WEST") {
    					return 7;
    				} else if (side == "EAST") {
    					if (typeof this.getBBox == "undefined") {
    						// JSDOM under nodejs
    						return -this.textContent.length * CHAR_WIDTH - CHAR_WIDTH / 2
    					}
    					return -this.getBBox().width - CHAR_WIDTH / 2;
    				} else if (side == "NORTH") {
    					return 0;
    				} else if (side == "SOUTH") {
    					return 0;
    				} else {
    					throw new Error(side);
    				}
    			})
    			.attr("y", PORT_HEIGHT * 0.75);

    		// spot input/output marker
    		portG.append("use")
    			.attr("href", getIOMarker);
    	}
    }

    /**
     * Container for node renderer instances.
     * This object initiates the node to renderer binding process in prepare()
     * and executes node rendering in render()
     */
    class NodeRendererContainer {
    	constructor() {
    		this.renderers = [];
    	}

    	// add new renderer
    	registerRenderer(renderer) {
    		var rs = this.renderers;
    		for (var i = 0; i < rs.length; i++) {
    			var r = rs[i];
    			if (r.constructor === GenericNodeRenderer) {
    				// insert custom renderer before GenericNodeRenderer
    				// to prevent GenericNodeRenderer.selector from prematurely halting renderers.some
    				rs.splice(i, 0, renderer);
    				return;
    			}
    		}
    		rs.push(renderer);
    	}

    	// Bind node to renderer recursively
    	prepare(node) {
    		var r = null;
    		this.renderers.some(function(ren) {
    			if (ren.selector(node))
    				r = ren;
    			return r != null;
    		});
    		if (r == null) {
    			throw new Error("Can not resolve renderer for node " + node);
    		}
    		node.hwMeta.renderer = r;
    		r.prepare(node);
    		var prep = this.prepare.bind(this);
    		if (node.children) {
    			node.children.forEach(prep);
    		}
    		if (node._children) {
    			node._children.forEach(prep);
    		}
    	}

    	// Render all nodes using selected renderer
    	render(root, nodeG) {
    		var renderers = this.renderers;
    		var nodesForRenderer = renderers.map(() => []);
    		nodeG.each(function(d) {
    			var n = this;
    			renderers.forEach(function(r, i) {
    				if (d.hwMeta.renderer === r) {
    					nodesForRenderer[i].push(n);
    				}
    			});
    		});
    		nodesForRenderer.forEach(function(nodes, i) {
    			if (nodes.length) {
    				nodes = d3__namespace.selectAll(nodes);
    				renderers[i].render(root, nodes);
    			}
    		});
    	}
    }

    /**
     * Library of functions which creates shapes of operator nodes (gate symbols)
     **/


    /**
     * Draw a circle for arithmetic nodes
     */
    function nodeCircle(root) {
      root.append("circle")
        .attr("r", "12.5")
        .attr("cx", "12.5")
        .attr("cy", "12.5");
    }

    /**
     * Draw a negation circle for nodes like NOT, NAND, NOR, etc...
     */
    function negationCircle(root, x, y) {
      root.append("circle")
        .attr("cx", x)
        .attr("cy", y)
        .attr("r", "3");
    }

    function nodeCircleWithText(root, text) {
      // width="25" height="25"
      var tl = text.length;
      if (tl > 2) {
        throw new Error("Text too big for small node circle");
      }
      var x = 8;
      if (tl === 2)
        x = 4;

      nodeCircle(root);
      root.append("text")
        .attr("x", x)
        .attr("y", 16)
        .text(text);
    }

    function nodeBiggerCircleWithText(root, text) {
      // width="25" height="25"
      var tl = text.length;
      if (tl > 6) {
        throw new Error("Text too big for small node circle");
      }
      var x = 6;

      root.append("circle")
        .attr("r", "25")
        .attr("cx", "25")
        .attr("cy", "25");
      root.append("text")
        .attr("x", x)
        .attr("y", 28.5)
        .text(text);
    }
    function operatorBox(root) {
      root.append("rect")
        .attr("width", "25")
        .attr("height", "25")
        .attr("x", "0")
        .attr("y", "0");
    }


    /**
     * Draw a AND gate symbol
     */
    function AND(root, addName = true) {
      // width="30" height="25"
      var g = root.append("g");
      g.append("path")
        .attr("d", "M0,0 L0,25 L15,25 A15 12.5 0 0 0 15,0 Z");
      g.attr("transform", "scale(0.8) translate(0, 3)");
      if (addName)
        root.append("text")
          .attr("x", 8)
          .attr("y", 16)
          .text("&");
      return g;
    }

    /**
     * Draw a NAND gate symbol
     */
    function NAND(root) {
      // width="30" height="25"
      AND(root, false);
      negationCircle(root, 34, 12.5);
    }


    var OR_SHAPE_PATH = "M3,0 A30 25 0 0 1 3,25 A30 25 0 0 0 33,12.5 A30 25 0 0 0 3,0 z";
    /**
     * Draw a OR gate symbol
     */
    function OR(root, addName = true) {
      // width="30" height="25"
      var g = root.append("g");
      g.append("path")
        .attr("d", OR_SHAPE_PATH);
      g.attr("transform", "scale(0.8) translate(0, 3)");
      if (addName)
        root.append("text")
          .attr("x", 5)
          .attr("y", 16)
          .text("or");
      return g;
    }

    /**
     * Draw a NOR gate symbol
     */
    function NOR(root) {
      // width="33" height="25"
      var g = OR(root, false);
      g.append("circle")
        .attr("cx", "34")
        .attr("cy", "12.5")
        .attr("r", "3");
      root.append("text")
        .attr("x", 5)
        .attr("y", 16)
        .text("!|");
    }


    /**
     * Draw a XOR gate symbol
     */
    function XOR(root) {
      var g = OR(root, false);
      g.append("path")
        .attr("d", "M0,0 A30 25 0 0 1 0,25");
      root.append("text")
        .attr("x", 8)
        .attr("y", 16)
        .text("^");

      return g;
    }


    /**
     * Draw a NXOR gate symbol
     */
    function NXOR(root) {
      // width="33" height="25"
      var g = XOR(root);
      negationCircle(g, 35, 12.5);
      root.append("text")
        .attr("x", 4)
        .attr("y", 16)
        .text("!^");
    }

    /**
     * Draw a NOT gate symbol
     */
    function NOT(root) {
      // width="30" height="20"
      root.append("path")
        .attr("d", "M0,2.5 L0,22.5 L20,12.5 Z");
      negationCircle(root, 23, 12.5);
      root.append("text")
        .attr("x", 2)
        .attr("y", 16)
        .text("!");
    }

    /**
     * Draw a FF register symbol
     */
    function FF(root) {
      // width="25" height="25"
      operatorBox(root);

      root.append("path")
        .attr("d", "M0,2 L5,7 L0,12");

      root.append("text")
        .attr("x", 5)
        .attr("y", 16)
        .text("FF");
    }

    function FF_ARST(root, arstPolarity, clkPolarity) {
        root.append("rect")
            .attr("width", "40")
            .attr("height", "50")
            .attr("x", "0")
            .attr("y", "0");

        //component name
        root.append("text")
            .attr("x", 7)
            .attr("y", 16)
            .text("ADFF");

        //triangle
        root.append("path")
            .attr("d", "M0,7.5 L6,12.5 L0,17.5 z");

        if (!clkPolarity) {
            root.append("circle")
                .attr("cx", 1)
                .attr("cy", 12.5)
                .attr("r", "1.5")
                .style("fill", "white");
        }

        if (!arstPolarity) {
            root.append("circle")
                .attr("cx", 1)
                .attr("cy", 25)
                .attr("r", "1.5")
                .style("fill", "white");
        }

        root.append("text")
            .attr("x", 4)
            .attr("y", 27.5)
            .style("font-size", "8px")
            .text("ARST");
    }
    function DLATCH(root, enPolarity) {
      root.append("rect")
          .attr("width", "50")
          .attr("height", "25")
          .attr("x", "0")
          .attr("y", "0");

      root.append("text")
          .attr("x", 3)
          .attr("y", 12)
          .text("DLATCH");

      if (!enPolarity) {
        root.append("circle")
            .attr("cx", 1)
            .attr("cy", 16.5)
            .attr("r", "1.5")
            .style("fill", "white");
      }

      root.append("text")
          .attr("x", 4)
          .attr("y", 19)
          .style("font-size", "8px")
          .text("en");
    }
    function RISING_EDGE(root) {
      // width="25" height="25"
      operatorBox(root);

      root.append("path")
        .attr("d", "M5,20 L12.5,20 L12.5,5 L20,5");
    }

    function FALLING_EDGE(root) {
      // width="25" height="25"
      operatorBox(root);

      root.append("path")
        .attr("d", "M5,5 L12.5,5 L12.5,20 L20,20");
    }


    const DEFAULT_NODE_SIZE = [25, 25];
    const SHAPES = {
      "NOT": [NOT, DEFAULT_NODE_SIZE],

      "AND": [AND, DEFAULT_NODE_SIZE],
      "NAND": [NAND, DEFAULT_NODE_SIZE],
      "OR": [OR, DEFAULT_NODE_SIZE],
      "NOR": [NOR, DEFAULT_NODE_SIZE],
      "XOR": [XOR, DEFAULT_NODE_SIZE],
      "NXOR": [NXOR, DEFAULT_NODE_SIZE],

      "RISING_EDGE": [RISING_EDGE, DEFAULT_NODE_SIZE],
      "FALLING_EDGE": [FALLING_EDGE, DEFAULT_NODE_SIZE],

      "ADD": [function ADD(root) {
        nodeCircleWithText(root, "+");
      }, DEFAULT_NODE_SIZE],
      "SUB": [function SUB(root) {
        nodeCircleWithText(root, "-");
      }, DEFAULT_NODE_SIZE],

      "EQ": [function EQ(root) {
        nodeCircleWithText(root, "=");
      }, DEFAULT_NODE_SIZE],
      "NE": [function NE(root) {
        nodeCircleWithText(root, "!=");
      }, DEFAULT_NODE_SIZE],
      "LT": [function LT(root) {
        nodeCircleWithText(root, "<");
      }, DEFAULT_NODE_SIZE],
      "LE": [function LE(root) {
        nodeCircleWithText(root, "<=");
      }, DEFAULT_NODE_SIZE],
      "GE": [function GE(root) {
        nodeCircleWithText(root, ">=");
      }, DEFAULT_NODE_SIZE],
      "GT": [function GT(root) {
        nodeCircleWithText(root, ">");
      }, DEFAULT_NODE_SIZE],
      "SHL": [function GT(root) {
        nodeCircleWithText(root, "<<");
      }, DEFAULT_NODE_SIZE],
      "SHR": [function GT(root) {
        nodeCircleWithText(root, ">>");
      }, DEFAULT_NODE_SIZE],
      "SHIFT": [function GT(root) {
        nodeBiggerCircleWithText(root, "<<,>>");
      }, [50, 50]],
      "MUL": [function GT(root) {
        nodeCircleWithText(root, "*");
      }, DEFAULT_NODE_SIZE],
      "DIV": [function GT(root) {
        nodeCircleWithText(root, "/");
      }, DEFAULT_NODE_SIZE],

      "FF": [FF, DEFAULT_NODE_SIZE],
      "FF_ARST_clk0_rst0": [(root) => { return FF_ARST(root, false, false)}, [40, 50]],
      "FF_ARST_clk1_rst1": [(root) => { return FF_ARST(root, true, true)}, [40, 50]],
      "FF_ARST_clk0_rst1": [(root) => { return FF_ARST(root, true, false)}, [40, 50]],
      "FF_ARST_clk1_rst0": [(root) => { return FF_ARST(root, false, true)}, [40, 50]],
      "DLATCH_en0": [(root) => {return DLATCH(root,false)}, [50, 25]],
      "DLATCH_en1": [(root) => {return DLATCH(root,true)}, [50, 25]],


    };

    /*
     * Render a operator node using predefined shape
     * */
    class OperatorNodeRenderer extends GenericNodeRenderer {
    	constructor(schematic) {
    		super(schematic);
    		this.SHAPES = SHAPES;
    		this._defsAdded = false;
    	}

    	prepare(node) {
    		if (!this._defsAdded) {
    			var defs = this.schematic.defs;
    			var SHAPES = this.SHAPES;
    			for (const [name, [constructorFn, _]] of Object.entries(SHAPES)) {
    				this.addShapeToDefs(defs, name, constructorFn);
    			}
    			this._defsAdded = true;
    		}
    		[node.width, node.height] =  this.SHAPES[node.hwMeta.name][1];
    	}

    	selector(node) {
    		return node.hwMeta.cls === "Operator" && typeof this.SHAPES[node.hwMeta.name] !== "undefined";
    	}

    	addShapeToDefs(defs, id, shape) {
    		var cont = defs.append("g")
    		    .attr("id", id)
                .attr("class", "d3-hwschematic node-operator");
            // [note] we need to add d3-hwschematic as well because object in refs are recognized as outside objects when useds
    		shape(cont);
    	}

    	/**
    	 * Render svg of node
    	 * 
    	 * @param root root svg element where nodes should be rendered
    	 * @param nodeG svg g for each node with data binded
    	 * */
    	render(root, nodeG) {
    		// apply node positions
    		nodeG.attr("transform", function(d) {
    			if (typeof d.x === "undefined" || typeof d.x === "undefined") {
    				throw new Error("Node with undefined position", d);
    			}
    			return "translate(" + d.x + " " + d.y + ")"
             })
            .attr("class", (d) => {
    			if (d.hwMeta.cssClass)
    			    return 'node ' + d.hwMeta.cssClass;
    			else 
    				return 'node';
    			})
            .attr("style", (d) => d.hwMeta.cssStyle)
    		.append("use")
    		.attr("href", function(d) {
    			return "#" + d.hwMeta.name
    		});
    	}
    }

    /**
     * Draw a multiplexer operator symbol
     */ 
    function MUX_SHAPE(root) {
        // width="20" height="40"
        root.append("path")
          .attr("d","M0,0 L20,10 L20,30 L0,40 Z");
    }

    class MuxNodeRenderer extends GenericNodeRenderer {
        constructor(schematic) {
            super(schematic);
            this.DEFULT_NODE_SIZE = [20, 40];
    		this._defsAdded = false;
        }

        prepare(node) {
    		if (!this._defsAdded) {
    	        var defs = this.schematic.defs;
    	        this.addShapeToDefs(defs);
    			this._defsAdded = true;
    		}
            node.width = this.DEFULT_NODE_SIZE[0];
            node.height = this.DEFULT_NODE_SIZE[1];
        }
        
        selector(node) {
            return node.hwMeta.cls == "Operator" && (
            		node.hwMeta.name === "MUX" ||
            		node.hwMeta.name === "LATCHED_MUX"
            );
        }
        
        addShapeToDefs(defs) {
            var cont = defs.append("g");
            cont.attr("id", "MUX");
            cont.attr("class", "d3-hwschematic node-operator");
            MUX_SHAPE(cont);
            
            var cont = defs.append("g");
            cont.attr("id", "LATCHED_MUX");
            cont.attr("class", "d3-hwschematic node-operator");
            MUX_SHAPE(cont);
            cont.append("text")
              .text("LA")
              .attr("y", "10")
              .attr("x", "10")
              .attr("style", "writing-mode: tb;");
        }
            
        /**
         * Render svg of node
         * 
         * @param root root svg element where nodes should be rendered
         * @param nodeG svg g for each node with data binded
         * */
        render(root, nodeG) {
            // apply node positions
            nodeG.attr("transform", function(d) {
                if (typeof d.x === "undefined" || typeof d.x === "undefined") {
                    throw new Error("Node with undefined position", d);
                }
                return "translate(" + d.x + " " + d.y + ")"
            })
            .attr("class", (d) => {
    			if (d.hwMeta.cssClass)
    			    return 'node ' + d.hwMeta.cssClass;
    			else 
    				return 'node';
    			})
            .attr("style", (d) => d.hwMeta.cssStyle)
            .append("use")
            .attr("href", function (d) {
                return "#" + d.hwMeta.name;
            });
            

        }
    }

    class SliceNodeRenderer extends GenericNodeRenderer {
    	selector(node) {
    		return node.hwMeta.name === "SLICE" || node.hwMeta.name === "CONCAT";
    	}
    	
    	getNodeLabelWidth(node) {
    		return 0;
    	}
    	
    	render(root, nodeG) {
            nodeG
                .attr("class", (d) => {
                  if (d.hwMeta.cssClass)
                      return 'node ' + d.hwMeta.cssClass;
                  else 
                    return 'node';
                  })
                .attr("style", (d) => d.hwMeta.cssStyle);
            
            // spot node main body and set dimensions and style of node
            nodeG.append("rect")
               .attr("width", function(d) { return d.width })
               .attr("height", function(d) { return d.height })
               .attr("class",  "node")
               .attr("rx", 5) // rounded corners
               .attr("ry", 5);

            // black thick line 
            nodeG.append("rect")
              .attr("x", function (d) {
            	  if (d.hwMeta.name == "SLICE") {
            		  return 0;
            	  } else {
            		  return d.width - 3;
            	  }
              })
              .attr("width", "3")
              .attr("height", function(d) { return d.height })
              .attr("style", "fill:black;"); 

            // apply node positions
            nodeG.attr("transform", function(d) {
                  if (typeof d.x === "undefined" || typeof d.x === "undefined") {
                      throw new Error("Node with undefined position", d);
                  }
                  return "translate(" + d.x + " " + d.y + ")"
              });
            
            this.renderPorts(nodeG);
    	}
    }

    const RUNNING_IN_NODE = (typeof require !== "undefined");
    const NO_LAYOUT = "org.eclipse.elk.noLayout";
    // kgraph properties that shall be copied
    const KGRAPH_KEYS = [
    	'x', 'y',
    	'width', 'height',
    	"sections",
    	'sourcePoint',
    	'targetPoint',
    	'junctionPoints',
    	'properties'
    ].reduce(function(p, c) { p[c] = 1; return p; }, {});

    /**
      * Webworker creates new graph object and layout props has to be copied back
      * to original graph
      * 
      * @param srcGraph:
      *            new graph from ELK worker
      * @param dstGraph:
      *            original graph provided by user
      * @param d3Objs:
      *            {str(dst obj id): dst obj}
      */
    function copyElkProps(srcGraph, dstGraph, d3Objs) {
    	// init d3Objs
    	d3Objs[dstGraph.id] = dstGraph;
    	(dstGraph.edges || []).forEach(function(e) {
    		if (e.id in d3Objs && d3Objs[e.id] !== e)
    			throw new Error("Duplicit edge" + e.id);
    		d3Objs[e.id] = e;
    	});
    	(dstGraph.children || []).forEach(function(n) {
    		d3Objs[n.id] = n;
    	});
    	(dstGraph.ports || []).forEach(function(p) {
    		d3Objs[p.id] = p;
    	});

    	// copy props from this node
    	copyProps(srcGraph, dstGraph);
    	(srcGraph.ports || []).forEach(function(p) {
    		copyProps(p, d3Objs[p.id]);
    	});
    	(srcGraph.labels || []).forEach(function(l, i) {
    		copyProps(l, dstGraph.labels[i]);
    	});
    	// copy props from edges in this node
    	(srcGraph.edges || []).forEach(function(e) {
    		var l = d3Objs[e.id];
    		copyProps(e, l);
    		copyProps(e.source, l.source);
    		copyProps(e.target, l.target);
    		// make sure the bendpoint array is valid
    		l.bendPoints = e.bendPoints || [];
    	});
    	// copy props of children
    	(srcGraph.children || []).forEach(function(n) {
    		copyElkProps(n, d3Objs[n.id], d3Objs);
    	});
    }
    function copyProps(src, dst) {
    	var keys = KGRAPH_KEYS;
    	for (var k in src) {
    		if (keys[k]) {
    			dst[k] = src[k];
    		}
    	}
    }

    /**
      * Convert section from ELK json to svg path string
      */
    function section2svgPath(section) {
    	var pathBuff = ["M", section.startPoint.x, section.startPoint.y];
    	if (section.bendPoints)
    		section.bendPoints.forEach(function(bp, i) {
    			pathBuff.push("L");
    			pathBuff.push(bp.x);
    			pathBuff.push(bp.y);
    		});

    	pathBuff.push("L");
    	pathBuff.push(section.endPoint.x);
    	pathBuff.push(section.endPoint.y);
    	return pathBuff.join(" ")
    }

    /**
      * Set the scale to value so
      * the available space is used to it's maximum.
      */
    function zoomToFit(node, width, height, g) {
    	var xOffset = -node.x;
    	var yOffset = -node.y;
    	var w = node.width || 1;
    	var h = node.height || 1;
    	// scale everything so that it fits the specified size
    	var scale = Math.min(width / w, height / h);
    	// centering
    	xOffset += ((width / scale - node.width) / 2);
    	yOffset += ((height / scale - node.height) / 2);

    	// if a transformation group was specified we
    	// perform a 'zoomToFit'
    	var t = d3__namespace.zoomTransform(g.node());
    	t.k = scale;
    	t.x = xOffset * scale;
    	t.y = yOffset * scale;
    	if (!RUNNING_IN_NODE) {
    		g = g.transition()
    			.duration(200);
    	}
    	g.attr("transform", t);
    }

    function isDescendant(node, child) {
    	var parent = child.parent;
    	while (parent) {
    		if (parent == node) {
    			return true;
    		}
    		parent = parent.parent;
    	}
    	return false;
    }
    function toAbsolutePositionsEdges(n, nodeMap) {
    	// edges
    	(n.edges || []).forEach(function(e) {
    		// transform edge coordinates to absolute coordinates. Note that
    		// node coordinates are already absolute and that
    		// edge coordinates are relative to the source node's parent node
    		// (unless the target node is a descendant of the source node)
    		var srcNode = nodeMap[e.source];
    		var tgtNode = nodeMap[e.target];
    		var relative = isDescendant(srcNode, tgtNode) ?
    			srcNode : srcNode.parent;

    		var offset = { x: 0, y: 0 };
    		if (relative) {
    			offset.x = relative.x || 0;
    			offset.y = relative.y || 0;
    		}
    		if (relative.padding) {
    			offset.x += relative.padding.left || 0;
    			offset.y += relative.padding.top || 0;
    		}
    		if (e.sections)
    			e.sections.forEach(function(s) {
    				// ... and apply it to the edge
    				if (s.startPoint) {
    					s.startPoint.x += offset.x;
    					s.startPoint.y += offset.y;
    				}
    				if (s.endPoint) {
    					s.endPoint.x += offset.x;
    					s.endPoint.y += offset.y;
    				}
    				(s.bendPoints || []).forEach(function(bp) {
    					bp.x += offset.x;
    					bp.y += offset.y;
    				});
    			});
    		if (e.junctionPoints)
    			e.junctionPoints.forEach(function(jp) {
    				jp.x += offset.x;
    				jp.y += offset.y;
    			});
    	});
    	// children
    	(n.children || []).forEach(function(c) {
    		toAbsolutePositionsEdges(c, nodeMap);
    	});
    }
    function toAbsolutePositions(n, offset, nodeMap) {
    	n.x = (n.x || 0) + offset.x;
    	n.y = (n.y || 0) + offset.y;
    	nodeMap[n.id] = n;
    	// the offset for the children has to include padding
    	var childOffset = { x: n.x, y: n.y };
    	if (n.padding) {
    		childOffset.x += n.padding.left || 0;
    		childOffset.y += n.padding.top || 0;
    	}
    	// children
    	(n.children || []).forEach(function(c) {
    		c.parent = n;
    		toAbsolutePositions(c, childOffset, nodeMap);
    	});
    }


    /**
      * Clean all layout possitions from nodes, nets and ports
      */
    function cleanLayout(n) {
    	delete n.x;
    	delete n.y;
    	(n.ports || []).forEach(function(p) {
    		delete p.x;
    		delete p.y;
    	});
    	(n.edges || []).forEach(function(e) {
    		delete e.sections;
    		delete e.junctionPoints;
    	});
    	(n.children || []).forEach(function(c) {
    		cleanLayout(c);
    	});
    }

    function renderLinks(root, edges) {
        let junctionPoints = [];

        let link = root.selectAll(".link")
          .data(edges)
          .enter()
          .append("path")
          .attr("class", "link")
          .attr("d", function(d) {
              if (!d.sections) {
                  d._svgPath = "";
                  return "";
              }
              if (d.bendpoints || d.sections.length > 1) {
                  throw new Error("NotImplemented");
              }
              if(d.junctionPoints)
                  d.junctionPoints.forEach(function (jp) {
                      junctionPoints.push(jp);
                  });
              d._svgPath = section2svgPath(d.sections[0]);
              return d._svgPath;
          });

        let linkWrap = root.selectAll(".link-wrap")
          .data(edges)
          .enter()
          .append("path")
          .attr("class", function (d) {
              let cssClass;
               if (d.hwMeta.parent) {
    	           cssClass = d.hwMeta.parent.hwMeta.cssClass;
               } else {
    	           cssClass = d.hwMeta.cssClass;
               }
               if (typeof cssClass !== 'undefined') {
    	           return "link-wrap " + cssClass;
               } else {
    	           return "link-wrap";
               }
          })
          .attr("style", function (d) {
               if (d.hwMeta.parent) {
    	           return d.hwMeta.parent.hwMeta.cssStyle;
               } else {
    	           return d.hwMeta.cssStyle
               }
          })
          .attr("d", function(d) {
              return d._svgPath;
          });

        let junctionPoint = root.selectAll(".junction-point")
          .data(junctionPoints)
          .enter()
          .append("circle")
          .attr("r", "3")
          .attr("cx", function(d) {
              return d.x;
          })
          .attr("cy", function(d) {
              return d.y;
          })
          .attr("class", "junction-point");

        return [link, linkWrap, junctionPoint];
    }

    class Tooltip {
      constructor(root) {
        let t = this.tooltip = document.createElement("div");
        t.className = "d3-hwschematic-tooltip";
        t.style.display = "none";
        t.style.possition = "absolute";
        root.appendChild(t);
      }
      
      show(evt, text) {
        let t = this.tooltip;
        t.style.display = "block";
        t.innerHTML = text;
        t.style.left = evt.pageX + 10 + 'px';
        t.style.top = evt.pageY + 10 + 'px';
      }
      
      hide() {
        this.tooltip.style.display = "none";
      }
    }

    function yosysTranslateIcons(node, cell) {
        let meta = node.hwMeta;
        const t = cell.type;

        if (t === "$mux" || t === "$pmux") {
            meta.cls = "Operator";
            meta.name = "MUX";
        } else if (t === "$gt") {
            meta.cls = "Operator";
            meta.name = "GT";
        } else if (t === "$lt") {
            meta.cls = "Operator";
            meta.name = "LT";
        } else if (t === "$ge") {
            meta.cls = "Operator";
            meta.name = "GE";
        } else if (t === "$le") {
            meta.cls = "Operator";
            meta.name = "LE";
        } else if (t === "$not" || t === "$logic_not") {
            meta.cls = "Operator";
            meta.name = "NOT";
        } else if (t === "$logic_and" || t === "$and") {
            meta.cls = "Operator";
            meta.name = "AND";
        } else if (t === "$logic_or" || t === "$or") {
            meta.cls = "Operator";
            meta.name = "OR";
        } else if (t === "$xor") {
            meta.cls = "Operator";
            meta.name = "XOR";
        } else if (t === "$eq") {
            meta.cls = "Operator";
            meta.name = "EQ";
        } else if (t === "$ne") {
            meta.cls = "Operator";
            meta.name = "NE";
        } else if (t === "$add") {
            meta.cls = "Operator";
            meta.name = "ADD";
        } else if (t === "$sub") {
            meta.cls = "Operator";
            meta.name = "SUB";
        } else if (t === "$mul") {
            meta.cls = "Operator";
            meta.name = "MUL";
        } else if (t === "$div") {
            meta.cls = "Operator";
            meta.name = "DIV";
        } else if (t === "$slice") {
            meta.cls = "Operator";
            meta.name = "SLICE";
        } else if (t === "$concat") {
            meta.cls = "Operator";
            meta.name = "CONCAT";
        } else if (t === "$adff") {
            meta.cls = "Operator";
            let arstPolarity = cell.parameters["ARST_POLARITY"];
            let clkPolarity = cell.parameters["CLK_POLARITY"];
            if (clkPolarity && arstPolarity) {
                meta.name = "FF_ARST_clk1_rst1";
            } else if (clkPolarity) {
                meta.name = "FF_ARST_clk1_rst0";
            } else if (arstPolarity) {
                meta.name = "FF_ARST_clk0_rst1";
            } else {
                meta.name = "FF_ARST_clk0_rst0";
            }
        } else if (t === "$dff") {
            meta.cls = "Operator";
            meta.name = "FF";
        } else if (t === "$shift" || t === "$shiftx") {
            meta.cls = "Operator";
            meta.name = "SHIFT";
        } else if (t === "$dlatch") {
            meta.cls = "Operator";
            let enPolarity = cell.parameters["EN_POLARITY"];
            if (enPolarity) {
                meta.name = "DLATCH_en1";
            } else {
                meta.name = "DLATCH_en0";

            }
        }
    }

    function getPortSide(portName, direction, nodeName) {
        if (direction === "input" && nodeName === "MUX" && portName === "S") {
            return "SOUTH";
        }
        if (direction === "output") {
            return "EAST";
        }
        if (direction === "input") {
            return "WEST";
        }
        throw new Error("Unknown direction " + direction);
    }

    function orderClkAndRstPorts(node) {
        let index = 0;
        for (let port of node.ports) {
            let dstIndex = index;
            if (port.hwMeta.name === "CLK") {
                dstIndex = node.ports.length - 1;
            } else if (port.hwMeta.name === "ARST") {
                dstIndex = node.ports.length - 2;
            }
            if (index !== dstIndex) {
                let otherPort = node.ports[dstIndex];
                node.ports[dstIndex] = port;
                node.ports[index] = otherPort;
                otherPort.properties.index = port.properties.index;
                port.properties.index = dstIndex;
            }
            ++index;
        }
    }

    function iterNetnameBits(netnames, fn) {
        for (const [netname, netObj] of Object.entries(netnames)) {
            for (const bit of netObj.bits) {
                fn(netname, bit, Number.isInteger(bit), isConst(bit));
            }
        }
    }

    function getNetNamesDict(yosysModule) {
        let netnamesDict = {}; // yosys bits (netId): netname

        iterNetnameBits(yosysModule.netnames, (netname, bit, isInt, isStr) => {
            if (isInt) {
                netnamesDict[bit] = netname;
            } else if (!isStr) {
                throw new Error("Invalid type in bits: " + typeof bit);
            }
        });
        return netnamesDict;
    }

    function isConst(val) {
        return (typeof val === "string");
    }

    function getConstNodeName(nameArray) {
        let nodeName = nameArray.reverse().join("");
        nodeName = ["0b", nodeName].join("");
        if (nodeName.match(/^0b[01]+$/g)) {
            let res = BigInt(nodeName).toString(16);
            return ["0x", res].join("");
        }
        return nodeName;
    }

    function addEdge(edge, portId, edgeDict, startIndex, width) {
        let edgeArr = edgeDict[portId];
        if (edgeArr === undefined) {
            edgeArr = edgeDict[portId] = [];
        }
        edgeArr[startIndex] = [edge, width];
    }

    function getSourceAndTarget2(edge) {
        return [edge.sources, edge.targets, false, true];
    }

    function getSourceAndTargetForCell(edge) {
        return [edge.targets, edge.sources, true, false];
    }

    function getPortNameSplice(startIndex, width) {
        if (width === 1) {
            return `[${startIndex}]`;
        } else if (width > 1) {
            let endIndex = startIndex + width;
            return `[${endIndex}:${startIndex}]`;
        }

        throw new Error("Incorrect width" + width);

    }


    function hideChildrenAndNodes(node, yosysModule) {
        if (yosysModule !== null) {
            if (node.children.length === 0 && node.edges.length === 0) {
                delete node.children;
                delete node.edges;

            } else {
                node._children = node.children;
                delete node.children;
                node._edges = node.edges;
                delete node.edges;
            }
        }
    }


    function updatePortIndices(ports) {
        let index = 0;
        for (let port of ports) {
            port.properties.index = index;
            ++index;
        }
    }

    function dividePorts(ports) {
        let north = [];
        let east = [];
        let south = [];
        let west = [];

        for (let port of ports) {
            let side = port.properties.side;
            if (side === "NORTH") {
                north.push(port);
            } else if (side === "EAST") {
                east.push(port);
            } else if (side === "SOUTH") {
                south.push(port);
            } else if (side === "WEST") {
                west.push(port);
            } else {
                throw new Error("Invalid port side: " + side);
            }
        }

        return [north, east, south, west];
    }

    function convertPortOrderingFromYosysToElk(node) {
        let [north, east, south, west] = dividePorts(node.ports);
        node.ports = north.concat(east, south.reverse(), west.reverse());
        updatePortIndices(node.ports);

    }

    function getTopModuleName(yosysJson) {
        let topModuleName = undefined;
        for (const [moduleName, moduleObj] of Object.entries(yosysJson.modules)) {
            if (moduleObj.attributes.top) {
                topModuleName = moduleName;
                break;
            }
        }

        if (topModuleName === undefined) {
            throw new Error("Cannot find top");
        }

        return topModuleName;
    }

    function getNodePorts(node, dict){
        for (let port of node.ports) {
            dict[port.id] = port;
        }

    }
    function getPortIdToPortDict(node) {
        let PortIdToPortDict = {};

        getNodePorts(node, PortIdToPortDict);
        for (let child of node.children) {
            getNodePorts(child, PortIdToPortDict);
        }

        return PortIdToPortDict;
    }
    function getNodeIdToNodeDict(node,) {
        let nodeIdToNodeDict = {};
        nodeIdToNodeDict[node.id] = node;
        for (let child of node.children) {
            nodeIdToNodeDict[child.id] = child;
        }
        return nodeIdToNodeDict;
    }

    function getPortToEdgeDict(edges) {
        let portToEdgeDict = {};
        for (let edge of edges) {
            let targets = edge.targets;
            let sources = edge.sources;
            for (let [_, portId] of sources) {
                portToEdgeDict[portId] = edge;
            }

            for (let [_, portId] of targets) {
                portToEdgeDict[portId] = edge;
            }
        }
        return portToEdgeDict;
    }

    function getChildSourcePorts(ports) {
        let sourcePorts = [];
        for(let port of ports) {
            if (port !== undefined && port.direction === "INPUT") {
                sourcePorts.push(port);
            }
        }

        return sourcePorts;
    }

    function getEdgeTargetsIndex(targets, portId) {
        for(let i = 0; i < targets.length; ++i) {
            let target = targets[i];
            let [_, targetPortId] = target;

            if (portId === targetPortId) {
                return i;
            }
        }
        throw new Error("PortId was not found");

    }
    function aggregateTwoNodes(childSourcePorts, targetNode, targetPort, portIdToEdgeDict) {
        let i = 0;
        if (targetPort.properties.index !== 0) {
            throw new Error("Port index is not zero, need to regenerate indices in port labels");
        }
        for (let oldTargetPort of childSourcePorts) {
            let oldTargetPortId = oldTargetPort.id;
            let edge = portIdToEdgeDict[oldTargetPortId];
            let edgeTargetsIndex = getEdgeTargetsIndex(edge.targets, oldTargetPortId);
            edge.targets[edgeTargetsIndex][0] = targetNode.id;
            let newTargetPortIndex = targetPort.properties.index + i;
            if (i === 0) {
                targetNode.ports[newTargetPortIndex] = oldTargetPort;
            }
            else {
                targetNode.ports.splice(newTargetPortIndex, 0, oldTargetPort);
            }
            oldTargetPort.properties.index = newTargetPortIndex;
            ++i;
        }


    }

    function getChildTargetPortId(child) {
        for (let port of child.ports) {
            if (port !== undefined && port.direction === "OUTPUT")
            {
                return port.id;
            }
        }

        throw new Error("Concat child has no target");
    }

    function aggregate(node, childrenConcats, portIdToEdgeDict, portIdToPortDict, nodeIdToNodeDict) {
        let edgesToDelete = new Set();
        let childrenToDelete = new Set();

        for (let child of childrenConcats) {
            let childTargetPortId = getChildTargetPortId(child);
            let edge = portIdToEdgeDict[childTargetPortId];
            if (edge === undefined) {
                continue;
            }
            let targets = edge.targets;

           if (targets !== undefined && targets.length === 1) {
                let [nodeId, portId] = targets[0];
                let targetNode = nodeIdToNodeDict[nodeId];
                let targetPort = portIdToPortDict[portId];
                let childSourcePorts = getChildSourcePorts(child.ports);
                if (targetNode === undefined) {
                    throw new Error("Target node of target port is undefined");
                }
                if (targetNode.hwMeta.cls === "Operator" && targetNode.hwMeta.name === "CONCAT") {
                    aggregateTwoNodes(childSourcePorts, targetNode, targetPort, portIdToEdgeDict);
                    edgesToDelete.add(edge.id);
                    childrenToDelete.add(child.id);
                }
            }
        }
        node.children = node.children.filter((c) => {
            return !childrenToDelete.has(c.id);
        });
        node.edges = node.edges.filter((e) => {
            return !edgesToDelete.has(e.id);
        });
    }

    function fillConcats(children) {
        let concats = [];
        for (let child of children) {
            if (child.hwMeta.cls === "Operator" && child.hwMeta.name === "CONCAT") {
                concats.push(child);
            }
        }
        return concats;

    }

    function aggregateConcants(node) {
        let concats = fillConcats(node.children);
        let portIdToEdgeDict = getPortToEdgeDict(node.edges);
        let portIdToPortDict = getPortIdToPortDict(node);
        let nodeIdToNodeDict = getNodeIdToNodeDict(node);
        aggregate(node, concats, portIdToEdgeDict, portIdToPortDict, nodeIdToNodeDict);
    }

    class LNodeMaker {
        constructor(name, yosysModule, idCounter, yosysModules, hierarchyLevel, nodePortNames) {
            this.name = name;
            this.yosysModule = yosysModule;
            this.idCounter = idCounter;
            this.yosysModules = yosysModules;
            this.hierarchyLevel = hierarchyLevel;
            this.nodePortNames = nodePortNames;
            this.childrenWithoutPortArray = [];
            this.nodeIdToCell = {};
        }

        make() {
            if (this.name === undefined) {
                throw new Error("Name is undefined");
            }

            let node = this.makeNode(this.name);

            if (this.yosysModule) {
                // cell with module definition, load ports, edges and children from module def. recursively
                this.fillPorts(node, this.yosysModule.ports, (p) => {
                    return p.direction
                }, undefined);
                this.fillChildren(node);
                this.fillEdges(node);

                if (node.children !== undefined && node.children.length > 0) {
                    aggregateConcants(node);
                }

            }

            if (node.children !== undefined) {
                for (let child of node.children) {
                    convertPortOrderingFromYosysToElk(child);
                    if (child.hwMeta.cls === "Operator" && child.hwMeta.name.startsWith("FF")) {
                        orderClkAndRstPorts(child);
                    }
                }
            }

            if (this.hierarchyLevel > 1) {
                hideChildrenAndNodes(node, this.yosysModule);
            }

            node.hwMeta.maxId = this.idCounter - 1;
            return node;
        }
        makeNode(name) {
            let node = {
                "id": this.idCounter.toString(), //generate, each component has unique id
                "hwMeta": { // [d3-hwschematic specific]
                    "name": name, // optional str
                    "cls": "", // optional str
                    "maxId": 2, // max id of any object in this node used to avoid re-counting object if new object is generated
                },
                "properties": { // recommended renderer settings
                    "org.eclipse.elk.portConstraints": "FIXED_ORDER", // can be also "FREE" or other value accepted by ELK
                    "org.eclipse.elk.layered.mergeEdges": 1
                },
                "ports": [],    // list of LPort
                "edges": [],    // list of LEdge
                "children": [], // list of LNode
            };
            ++this.idCounter;
            return node;
        }

        fillPorts(node, ports, getPortDirectionFn, cellObj) {
            const isSplit = cellObj !== undefined && cellObj.type === "$slice";
            const isConcat = cellObj !== undefined && cellObj.type === "$concat";
            let portByName = this.nodePortNames[node.id];
            if (portByName === undefined) {
                portByName = {};
                this.nodePortNames[node.id] = portByName;
            }
            for (let [portName, portObj] of Object.entries(ports)) {
                let originalPortName = portName;
                if (isSplit || isConcat) {
                    if (portName === "Y") {
                        portName = "";
                    }
                    if (isSplit) {
                        if (portName === "A") {
                            portName = getPortNameSplice(cellObj.parameters.OFFSET, cellObj.parameters.Y_WIDTH);
                        }
                    } else if (isConcat) {
                        let par = cellObj.parameters;
                        if (portName === "A") {
                            portName = getPortNameSplice(0, par.A_WIDTH);
                        } else if (portName === "B") {
                            portName = getPortNameSplice(par.A_WIDTH, par.B_WIDTH);
                        }
                    }
                }
                let direction = getPortDirectionFn(portObj);
                this.makeLPort(node.ports, portByName, originalPortName, portName, direction, node.hwMeta.name);
            }
        }

        makeLPort(portList, portByName, originalName, name, direction, nodeName) {
            if (name === undefined) {
                throw new Error("Name is undefined");
            }

            let portSide = getPortSide(name, direction, nodeName);
            let port = {
                "id": this.idCounter.toString(),
                "hwMeta": { // [d3-hwschematic specific]
                    "name": name,
                },
                "direction": direction.toUpperCase(), // [d3-hwschematic specific] controls direction marker
                "properties": {
                    "side": portSide,
                    "index": 0 // The order is assumed as clockwise, starting with the leftmost port on the top side.
                    // Required only for components with "org.eclipse.elk.portConstraints": "FIXED_ORDER"
                },
                "children": [], // list of LPort, if the port should be collapsed rename this property to "_children"
            };
            port.properties.index = portList.length;
            portList.push(port);
            portByName[originalName] = port;
            ++this.idCounter;
            return port;
        }

        fillChildren(node) {
            // iterate all cells and lookup for modules and construct them recursively
            for (const [cellName, cellObj] of Object.entries(this.yosysModule.cells)) {
                let moduleName = cellObj.type; //module name
                let cellModuleObj = this.yosysModules[moduleName];
                let nodeBuilder = new LNodeMaker(cellName, cellModuleObj, this.idCounter, this.yosysModules,
                    this.hierarchyLevel + 1, this.nodePortNames);
                let subNode = nodeBuilder.make();
                this.idCounter = nodeBuilder.idCounter;
                node.children.push(subNode);
                yosysTranslateIcons(subNode, cellObj);
                this.nodeIdToCell[subNode.id] = cellObj;
                if (cellModuleObj === undefined) {
                    if (cellObj.port_directions === undefined) {
                        // throw new Error("[Todo] if modules does not have definition in modules and its name does not \
                        // start with $, then it does not have port_directions. Must add port to sources and targets of an edge")

                        this.childrenWithoutPortArray.push([cellObj, subNode]);
                        continue;
                    }
                    this.fillPorts(subNode, cellObj.port_directions, (p) => {
                        return p;
                    }, cellObj);
                }
            }
        }

        fillEdges(node) {

            let edgeTargetsDict = {};
            let edgeSourcesDict = {};
            let constNodeDict = {};
            let [edgeDict, edgeArray] = this.getEdgeDictFromPorts(
                node, constNodeDict, edgeTargetsDict, edgeSourcesDict);
            let netnamesDict = getNetNamesDict(this.yosysModule);

            function getPortName(bit) {
                return netnamesDict[bit];
            }

            for (let i = 0; i < node.children.length; i++) {
                const subNode = node.children[i];
                if (constNodeDict[subNode.id] === 1) {
                    //skip constants to iterate original cells
                    continue;
                }

                let cell = this.nodeIdToCell[subNode.id];
                if (cell.port_directions === undefined) {
                    continue;
                }
                let connections = cell.connections;
                let portDirections = cell.port_directions;


                if (connections === undefined) {
                    throw new Error("Cannot find cell for subNode" + subNode.hwMeta.name);
                }

                let portI = 0;
                let portByName = this.nodePortNames[subNode.id];
                for (const [portName, bits] of Object.entries(connections)) {
                    let portObj;
                    let direction;
                    if (portName.startsWith("$")) {
                        portObj = subNode.ports[portI++];
                        direction = portObj.direction.toLowerCase(); //use direction from module port definition
                    } else {
                        portObj = portByName[portName];
                        direction = portDirections[portName];
                    }

                    this.loadNets(node, subNode.id, portObj.id, bits, direction, edgeDict, constNodeDict,
                        edgeArray, getPortName, getSourceAndTargetForCell, edgeTargetsDict, edgeSourcesDict);

                }
            }
            // source null target null == direction is output

            for (const [cellObj, subNode] of this.childrenWithoutPortArray) {
                for (const [portName, bits] of Object.entries(cellObj.connections)) {
                    let port = null;
                    for (const bit of bits) {
                        let edge = edgeDict[bit];
                        if (edge === undefined) {
                            throw new Error("[Todo] create edge");
                        }
                        let edgePoints;
                        let direction;
                        if (edge.sources.length === 0 && edge.targets.length === 0) {
                            direction = "output";
                            edgePoints = edge.sources;
                        } else if (edge.sources.length === 0) {
                            // no sources -> add as source
                            direction = "output";
                            edgePoints = edge.sources;
                        } else {
                            direction = "input";
                            edgePoints = edge.targets;
                        }

                        if (port === null) {
                            let portByName = this.nodePortNames[subNode.id];
                            if (portByName === undefined) {
                                portByName = {};
                                this.nodePortNames[subNode.id] = portByName;
                            }
                            port = this.makeLPort(subNode.ports, portByName, portName, portName, direction, subNode.hwMeta.name);
                        }

                        edgePoints.push([subNode.id, port.id]);
                    }
                }

            }

            let edgeSet = {}; // [sources, targets]: true
            for (const edge of edgeArray) {
                let key = [edge.sources, null, edge.targets];
                if (!edgeSet[key]) // filter duplicities
                {
                    edgeSet[key] = true;
                    node.edges.push(edge);
                }
            }

        }

        getEdgeDictFromPorts(node, constNodeDict, edgeTargetsDict, edgeSourcesDict) {
            let edgeDict = {}; // yosys bits (netId): LEdge
            let edgeArray = [];
            let portsIndex = 0;
            for (const [portName, portObj] of Object.entries(this.yosysModule.ports)) {
                let port = node.ports[portsIndex];
                portsIndex++;

                function getPortName2() {
                    return portName;
                }

                this.loadNets(node, node.id, port.id, portObj.bits, portObj.direction,
                    edgeDict, constNodeDict, edgeArray, getPortName2, getSourceAndTarget2,
                    edgeTargetsDict, edgeSourcesDict);

            }
            return [edgeDict, edgeArray];
        }

        /*
         * Iterate bits representing yosys net names, which are used to get edges from the edgeDict.
         * If edges are not present in the dictionary, they are created and inserted into it. Eventually,
         * nodes are completed by filling sources and targets properties of LEdge.
         */
        loadNets(node, nodeId, portId, bits, direction, edgeDict, constNodeDict, edgeArray,
                 getPortName, getSourceAndTarget, edgeTargetsDict, edgeSourcesDict) {
            for (let i = 0; i < bits.length; ++i) {
                let startIndex = i;
                let width = 1;
                let bit = bits[i];
                let portName = getPortName(bit);
                let edge = edgeDict[bit];
                let netIsConst = isConst(bit);
                if (netIsConst || edge === undefined) {
                    // create edge if it is not in edgeDict
                    if (portName === undefined) {
                        if (!netIsConst) {
                            throw new Error("Netname is undefined");
                        }
                        portName = bit;
                    }
                    edge = this.makeLEdge(portName);
                    edgeDict[bit] = edge;
                    edgeArray.push(edge);
                    if (netIsConst) {
                        i = this.addConstNodeToSources(node, bits, edge.sources, i, constNodeDict);
                        width = i - startIndex + 1;
                    }
                }

                let [a, b, targetA, targetB] = getSourceAndTarget(edge);
                if (direction === "input") {
                    a.push([nodeId, portId]);
                    if (targetA) {
                        addEdge(edge, portId, edgeTargetsDict, startIndex, width);
                    } else {
                        addEdge(edge, portId, edgeSourcesDict, startIndex, width);
                    }
                } else if (direction === "output") {
                    b.push([nodeId, portId]);
                    if (targetB) {
                        addEdge(edge, portId, edgeTargetsDict, startIndex, width);
                    } else {
                        addEdge(edge, portId, edgeSourcesDict, startIndex, width);
                    }
                } else {
                    throw new Error("Unknown direction " + direction);
                }
            }
        }

        makeLEdge(name) {
            if (name === undefined) {
                throw new Error("Name is undefined");
            }
            let edge = {
                "id": this.idCounter.toString(),
                "sources": [],
                "targets": [], // [id of LNode, id of LPort]
                "hwMeta": { // [d3-hwschematic specific]
                    "name": name, // optional string, displayed on mouse over
                }
            };
            ++this.idCounter;
            return edge;
        }

        addConstNodeToSources(node, bits, sources, i, constNodeDict) {
            let nameArray = [];
            for (i; i < bits.length; ++i) {
                let bit = bits[i];
                if (isConst(bit)) {
                    nameArray.push(bit);
                } else {
                    break;
                }
            }
            --i;
            // If bit is a constant, create a node with constant
            let nodeName = getConstNodeName(nameArray);
            let constSubNode;
            let port;
            [constSubNode, port] = this.addConstNode(node, nodeName, constNodeDict);
            sources.push([constSubNode.id, port.id]);
            return i;
        }

        addConstNode(node, nodeName, constNodeDict) {
            let port;

            let nodeBuilder = new LNodeMaker(nodeName, undefined, this.idCounter, null,
                this.hierarchyLevel + 1, this.nodePortNames);
            let subNode = nodeBuilder.make();
            this.idCounter = nodeBuilder.idCounter;

            let portByName = this.nodePortNames[subNode.id] = {};
            port = this.makeLPort(subNode.ports, portByName, "O0", "O0", "output", subNode.hwMeta.name);
            node.children.push(subNode);
            constNodeDict[subNode.id] = 1;

            return [subNode, port];
        }


    }

    function yosys(yosysJson) {
        let nodePortNames = {};
        let rootNodeBuilder = new LNodeMaker("root", null, 0, null, 0, nodePortNames);
        let output = rootNodeBuilder.make();
        let topModuleName = getTopModuleName(yosysJson);

        let nodeBuilder = new LNodeMaker(topModuleName, yosysJson.modules[topModuleName], rootNodeBuilder.idCounter,
            yosysJson.modules, 1, nodePortNames);
        let node = nodeBuilder.make();
        output.children.push(node);
        output.hwMeta.maxId = nodeBuilder.idCounter - 1;
        //yosysTranslateIcons(output);
        //print output to console
        //console.log(JSON.stringify(output, null, 2));

        return output;
    }

    function hyperEdgeListToEdges(eList, newEdges, idOffset) {
    	for (let ei = 0; ei < eList.length; ei++) {
    		let e = eList[ei];
    		let isHyperEdge = typeof e.sources !== "undefined";
    		if (isHyperEdge) {
    			let src;
    			let dst;
    		    if (e.sources.length === 1 && e.targets.length === 1) {
        		    src = e.sources[0];
                    dst = e.targets[0];
                    e.source = src[0];
                    e.sourcePort = src[1];
                    e.target = dst[0];
                    e.targetPort = dst[1];
                    delete e.sources;
                    delete e.targets;
                    newEdges.push(e);
    		    } else {
        			for (let s = 0; s < e.sources.length; s++) {
        				src = e.sources[s];
        				for (let t = 0; t < e.targets.length; t++) {
        					dst = e.targets[t];
        					idOffset += 1;
        					newEdges.push({
        						"hwMeta": { "parent": e },
        						"id": "" + idOffset,
        						"source": src[0],
        						"sourcePort": src[1],
        						"target": dst[0],
        						"targetPort": dst[1],
        					});
        				}
        			}
    			}
    		} else {
    			newEdges.push(e);
    		}
    	}
    	return idOffset;
    }

    /**
     * Convert hyperEdges to edges in whole graph
     *
     * @param n root node
     * @param idOffset int, max id in graph, used for generating
     *                 of new edges from hyperEdges
     **/
    function hyperEdgesToEdges(n, idOffset) {
    	let newEdges;
    	if (n.edges) {
    		newEdges = [];
    		idOffset = hyperEdgeListToEdges(n.edges, newEdges, idOffset);
    		n.edges = newEdges;
    	}
    	if (n._edges) {
    		newEdges = [];
    		idOffset = hyperEdgeListToEdges(n._edges, newEdges, idOffset);
    		n._edges = newEdges;
    	}
    	if (n.children) {
    		for (let i = 0; i < n.children.length; i++) {
    			idOffset = hyperEdgesToEdges(n.children[i], idOffset);
    		}
    	}
    	if (n._children) {
    		for (let i = 0; i < n._children.length; i++) {
    			idOffset = hyperEdgesToEdges(n._children[i], idOffset);
    		}
    	}
    	return idOffset
    }

    /**
     * Get parent of net for net
     **/
    function getNet(e) {
    	if (typeof e.hwMeta.parent !== "undefined") {
    		return e.hwMeta.parent;
    	} else {
    		return e;
    	}
    }

    function initNodeParents(node, parent) {
    	node.hwMeta.parent = parent;
    	(node.children || []).forEach(function(n) {
    		initNodeParents(n, node);
    	});
    	(node._children || []).forEach(function(n) {
    		initNodeParents(n, node);
    	});

    }
    function expandPorts(node) {
    	let portList = [];
    	if (node.ports)
        	node.ports.forEach(function (port) {expandPorts4port(port, portList);});
    	//node.hwMeta.parent = parent;
    	node.ports = portList;
    	(node.children || node._children || []).forEach(function(n) {
    		expandPorts(n);
    	});
    }

    function expandPorts4port(port, portList){
        if (port.hwMeta.connectedAsParent) {
            return;
        }
    	portList.push(port);
    	(port.children || []).forEach(function(p) {
    		p.parent = port;
    		expandPorts4port(p, portList);
    	});

    }

    /*
      * Collect list of expanded nodes
      */
    function computeLayoutCacheKey(n, res) {
    	res.push(n.id);
    	if (n.children) {
    		n.children.forEach((d) => { computeLayoutCacheKey(d, res); });
    	}
    }

    /*
    * Store current state of layout
    */
    function serializeLayout(n) {
    	var res = {
    		"id": n.id,
    		"x": n.x,
    		"y": n.y,
    		"width": n.width,
    		"height": n.height,
    	};
    	if (n.ports) {
    		res["ports"] = n.ports.map(function(p) {
    			return {
    				"id": p.id,
    				"x": p.x,
    				"y": p.y,
    				"width": p.width,
    				"height": p.height,
    			};
    		});
    	}
    	if (n.edges) {
    		res["edges"] = n.edges.map(function(e) {
    			return {
    				"id": e.id,
    				"sections": e.sections,
    				"junctionPoints": e.junctionPoints
    			};
    		});
    	}
    	if (n.children) {
    		res["children"] = n.children.map(function(c) {
    			return serializeLayout(c)
    		});
    	}
    	return res;
    }

    // apply cached element positions and size
    function applyCachedState(n, state) {
    	if (n.id != state.id) {
    		throw new Error("Cached state not matching current data");
    	}
    	n.x = state.x;
    	n.y = state.y;
    	n.width = state.width;
    	n.height = state.height;
    	if (n.ports) {
    		state.ports.forEach(function(s, i) {
    			var p = n.ports[i];
    			if (p.id != s.id) {
    				throw new Error("Cached state not matching current data");
    			}
    			p.x = s.x;
    			p.y = s.y;
    			p.width = s.width;
    			p.height = s.height;
    		});
    	}
    	if (n.edges) {
    		state.edges.forEach(function(s, i) {
    			var p = n.edges[i];
    			if (p.id != s.id) {
    				throw new Error("Cached state not matching current data");
    			}
    			p.sections = s.sections;
    			p.junctionPoints = s.junctionPoints;
    		});
    	}
    	if (n.children) {
    		state.children.forEach(function(s, i) {
    			var c = n.children[i];
    			return applyCachedState(c, s);
    		});
    	}
    }

    class d3elk {
    	constructor() {
    		// containers
    		this.graph = {}; // internal (hierarchical graph)
    		this._options = {};
    		// dimensions
    		this.width = 0;
    		this.height = 0;
    		this._transformGroup = undefined;

    		// the layouter instance
    		this.layouter = new ELK__default["default"]({
    			algorithms: ['layered'],
    		});
    		this._invalidateCaches();
    	}

    	/**
    	  * Set or get the available area, the positions of the layouted graph are
    	  * currently scaled down.
    	  */
    	size(size) {
    		if (!arguments.length)
    			return [this.width, this.height];
    		var old_w = this.width;
    		var old_h = this.height;
    		this.width = size[0];
    		this.height = size[1];

    		if (this.graph != null) {
    			if (old_w !== this.width || old_h !== this.height) {
    				this._layoutCache = {};
    			}
    			this.graph.width = this.width;
    			this.graph.height = this.height;
    		}
    		return this;
    	};

    	/**
    	  * Sets the group used to perform 'zoomToFit'.
    	  */
    	transformGroup(g) {
    		if (!arguments.length)
    			return this._transformGroup;
    		this._transformGroup = g;
    		return this;
    	}

    	options(opts) {
    		if (!arguments.length)
    			return this._options;
    		this._options = opts;
    		return this;
    	}

    	/**
    	  * Start the layout process.
    	  */
    	start() {
    		var cacheKey = [];
    		computeLayoutCacheKey(this.graph, cacheKey);
    		var state = this._layoutCache[cacheKey];
    		var _this = this;
    		if (typeof state !== 'undefined') {
    			// load layout from cache
    			return new Promise((resolve, reject) => {
    				resolve();
    			}).then(
    				function() {
    					applyCachedState(_this.graph, state);
    				}
    			)
    		} else {
    			// run layouter
    			this._cleanLayout();
    			this._currentLayoutCacheKey = cacheKey;
    			return this.layouter.layout(
    				this.graph,
    				{ layoutOptions: this._options }
    			).then(
    				this._applyLayout.bind(this),
    				function(e) {
    					// Error while running elkjs layouter
    					_this._currentLayoutCacheKey = null;
    					throw e;
    				}
    			);
    		}
    	}

    	// get currently visible nodes
    	getNodes() {
    		if (this.__nodeCache != null)
    			return this.__nodeCache;

    		var queue = [this.graph],
    			nodes = [],
    			parent;

    		// note that svg z-index is document order, literally
    		while ((parent = queue.pop()) != null) {
    			if (!parent.properties[NO_LAYOUT]) {
    				nodes.push(parent);
    				(parent.children || []).forEach(function(c) {
    					queue.push(c);
    				});
    			}
    		}
    		this.__nodeCache = nodes;
    		return nodes;
    	}


    	// get currently visible ports
    	getPorts() {
    		if (this.__portsCache != null)
    			return this.__portsCache;

    		var ports = d3__namespace.merge(this.getNodes().map(function(n) {
    			return n.ports || [];
    		}));
    		this.__portsCache = ports;
    	}


    	// get currently visible edges
    	getEdges() {
    		if (this.__edgesCache != null)
    			return this.__edgesCache;

    		var edgesOfChildren = d3__namespace.merge(
    			this.getNodes()
    				.filter(function(n) {
    					return n.children;
    				})
    				.map(function(n) {
    					return n.edges || [];
    				})
            );

    		this.__edgesCache = edgesOfChildren;
    		return this.__edgesCache;
    	}

    	// bind graph data
    	kgraph(root) {
    		if (!arguments.length)
    			return this.graph;

    		var g = this.graph = root;
    		this._invalidateCaches();

    		if (!g.id)
    			g.id = "root";
    		if (!g.properties)
    			g.properties = { 'algorithm': 'layered' };
    		if (!g.properties.algorithm)
    			g.properties.algorithm = 'layered';
    		if (!g.width)
    			g.width = this.width;
    		if (!g.height)
    			g.height = this.height;

    		return this;
    	};
    	/**
    	  * If a top level transform group is specified, we set the scale to value so
      	  * the available space is used to it's maximum.
    	  */
    	zoomToFit(node) {
    		if (!this._transformGroup) {
    			return;
    		}
    		if (node === null) {
    			node = this.graph;
    		}
    		zoomToFit(node, this.width, this.height, this._transformGroup);
    	}

    	terminate() {
    		if (this.layouter)
    			this.layouter.terminateWorker();
    	}

    	/**
         * Clean all layout possitions from nodes, nets and ports
         */
    	_cleanLayout(n) {
    		if (!arguments.length)
    			var n = this.graph;
    		cleanLayout(n);
    		return this;
    	}
    	_invalidateCaches() {
    		// cached used to avoid execuiton of elkjs to resolve the layout of
    		// graph if executed previously with same input
    		// {sorted list of expanded node ids: {nodeId: {"x": ..., "y": ...,
    		// "ports": {portId: [x, y]}},
    		// edgeId: [points] }}
    		this._layoutCache = {};
    		this._currentLayoutCacheKey = null;

    		// {id(str): object from input graph} used to access graph objects by it's id
    		this._d3ObjMap = {};
    		this.markLayoutDirty();
    	};
    	markLayoutDirty() {
    		this.__nodeCache = null;
    		this.__portsCache = null;
    		this.__edgesCache = null;
    	}
    	/**
    	  * Apply layout for the kgraph style. Converts relative positions to
    	  * absolute positions.
    	  */
    	_applyLayout(kgraph) {
    		this.zoomToFit(kgraph);
    		var nodeMap = {};
    		// convert to absolute positions
    		toAbsolutePositions(kgraph, { x: 0, y: 0 }, nodeMap);
    		toAbsolutePositionsEdges(kgraph, nodeMap);
    		copyElkProps(kgraph, this.graph, this._d3ObjMap);
    		this._layoutCache[this._currentLayoutCacheKey] = serializeLayout(this.graph);

    		return this.graph;
    	}
    }

    function findGraph(nodeArray, nodeName) {
        for (let node of nodeArray) {
            if (node.hwMeta.name === nodeName) {
                return node
            }
        }
        throw new Error("Node is not found: " + nodeName);
    }

    function selectGraphRootByPath(graph, path) {
        let pathArray = path.split("/");
        let first = true;
        let newGraph = graph;
        for (let nodeName of pathArray) {
            if (first && nodeName === "") {
                first = false;
                continue;
            }
            newGraph = findGraph((newGraph.children || newGraph._children), nodeName);
        }
        if (graph !== newGraph) {
            //case if path has nonzero length
            //copy because we need to make graph immutable because we will need it later
            graph = Object.assign({}, graph);
            graph.children = [newGraph];
        }
        return graph;

    }

    function getNameOfEdge(e) {
        let name = "<tspan>unnamed</tspan>";
        if (e.hwMeta) {
            if (typeof e.hwMeta.name === "undefined") {
                let p = e.hwMeta.parent;
                let pIsHyperEdge = typeof p.sources !== "undefined";
                if (pIsHyperEdge && p.hwMeta) {
                    name = p.hwMeta.name;
                }
            } else {
                name = e.hwMeta.name;
            }
        }
        return name;
    }

    function toggleHideChildren(node) {
        let children;
        let nextFocusTarget;
        if (node.children) {
            // children are visible, will collapse
            children = node.children;
            nextFocusTarget = node.hwMeta.parent;
        } else {
            // children are hidden, will expand
            children = node._children;
            nextFocusTarget = node;
        }

        let tmpChildren = node.children;
        node.children = node._children;
        node._children = tmpChildren;
        let tmpEdges = node.edges;
        node.edges = node._edges;
        node._edges = tmpEdges;
        node.hwMeta.renderer.prepare(node);
        return [children, nextFocusTarget];
    }

    /**
     * HwScheme builds scheme diagrams after bindData(data) is called
     *
     * @param svg: root svg element where scheme will be rendered
     * @attention zoom is not applied it is only used for focusing on objects
     * @note do specify size of svg to have optimal result
     */
    class HwSchematic {
        constructor(svg) {
            // flag for performance debug
            this._PERF = false;
            // main svg element
            this.svg = svg;
            // default sizes of elements
            this.PORT_PIN_SIZE = [7, 13];
            this.PORT_HEIGHT = this.PORT_PIN_SIZE[1];
            this.CHAR_WIDTH = 7.55;
            this.CHAR_HEIGHT = 13;
            this.NODE_MIDDLE_PORT_SPACING = 20;
            this.MAX_NODE_BODY_TEXT_SIZE = [400, 400];
            // top, right, bottom, left
            this.BODY_TEXT_PADDING = [15, 10, 0, 10];
            svg.classed("d3-hwschematic", true);
            this.defs = svg.append("defs");
            this.root = svg.append("g");
            this.errorText = null;
            this._nodes = null;
            this._edges = null;

            // graph layouter to resolve positions of elements
            this.layouter = new d3elk();
            this.layouter
                .options({
                    edgeRouting: "ORTHOGONAL",
                })
                .transformGroup(this.root);

            // shared tooltip object
            this.tooltip = new Tooltip(document.getElementsByTagName('body')[0]);

            // renderer instances responsible for rendering of component nodes
            this.nodeRenderers = new NodeRendererContainer();
            addMarkers(this.defs, this.PORT_PIN_SIZE);
            let rs = this.nodeRenderers;
            rs.registerRenderer(new OperatorNodeRenderer(this));
            rs.registerRenderer(new MuxNodeRenderer(this));
            rs.registerRenderer(new SliceNodeRenderer(this));
            rs.registerRenderer(new GenericNodeRenderer(this));
        }

        widthOfText(text) {
            if (text) {
                return text.length * this.CHAR_WIDTH;
            } else {
                return 0;
            }
        }

        removeGraph() {
            this.root.selectAll("*").remove();
        }

        updateGlobalSize() {
            let width = parseInt(this.svg.style("width") || this.svg.attr("width"), 10);
            let height = parseInt(this.svg.style("height") || this.svg.attr("height"), 10);

            this.layouter
                .size([width, height]);
        }

        /**
         * Set bind graph data to graph rendering engine
         *
         * @return promise for this job
         */
        bindData(graph) {
            this.removeGraph();
            let postCompaction = "layered.compaction.postCompaction.strategy";
            if (!graph.properties[postCompaction]) {
                graph.properties[postCompaction] = "EDGE_LENGTH";
            }
            hyperEdgesToEdges(graph, graph.hwMeta.maxId);
            initNodeParents(graph, null);
            expandPorts(graph);

            if (this._PERF) {
                let t0 = new Date().getTime();
                this.nodeRenderers.prepare(graph);
                let t1 = new Date().getTime();
                console.log("> nodeRenderers.prepare() : " + (t1 - t0) + " ms");
            } else {
                // nodes are ordered, children at the end
                this.nodeRenderers.prepare(graph);
            }
            this.layouter
                .kgraph(graph);
            return this._draw();
        }
        /*
        * @returns subnode selected by path wrapped in a new root
        * */
        static selectGraphRootByPath(graph, path) {
            return selectGraphRootByPath(graph, path);
        }
        /*
         * Resolve layout and draw a component graph from layout data
         */
        _draw() {
            this.updateGlobalSize();

            let layouter = this.layouter;
            this._nodes = layouter.getNodes().slice(1); // skip root node
            this._edges = layouter.getEdges();
            let t0;
            if (this._PERF) {
                t0 = new Date().getTime();
            }
            let _this = this;
            return layouter.start()
                .then(
                    function (g) {
                        if (_this._PERF) {
                            let t1 = new Date().getTime();
                            console.log("> layouter.start() : " + (t1 - t0) + " ms");
                            t0 = t1;
                        }
                        _this._applyLayout(g);
                        if (_this._PERF) {
                            let t1 = new Date().getTime();
                            console.log("> HwSchematic._applyLayout() : " + (t1 - t0) + " ms");
                        }
                    },
                    function (e) {
                        // Error while running d3-elkjs layouter
                        throw e;
                    }
                );
        }

        /**
         * Draw a component graph from layout data
         */
        _applyLayout() {
            let root = this.root;

            let node = root.selectAll(".node")
                .data(this._nodes)
                .enter()
                .append("g");
            this.nodeRenderers.render(root, node);

            let _this = this;
            node.on("click", function (ev, d) {
                let [children, nextFocusTarget] = toggleHideChildren(d);
                if (!children || children.length === 0) {
                    return; // does not have anything to expand
                }
                _this.layouter.markLayoutDirty();
                _this.removeGraph();
                _this._draw().then(
                    function () {
                        _this.layouter.zoomToFit(nextFocusTarget);
                    },
                    function (e) {
                        // Error while applying of layout
                        throw e;
                    }
                );
            });

            this._applyLayoutLinks();
        }

        _applyLayoutLinks() {
            let _this = this;
            let edges = this._edges;

            let [link, linkWrap, _] = renderLinks(this.root, edges);
            // build netToLink
            let netToLink = {};
            edges.forEach(function (e) {
                netToLink[getNet(e).id] = {
                    "core": [],
                    "wrap": []
                };
            });
            linkWrap._groups.forEach(function (lg) {
                lg.forEach(function (l) {
                    let e = d3__namespace.select(l).data()[0];
                    netToLink[getNet(e).id]["wrap"].push(l);
                });
            });
            link._groups.forEach(function (lg) {
                lg.forEach(function (l) {
                    let e = d3__namespace.select(l).data()[0];
                    netToLink[getNet(e).id]["core"].push(l);
                });
            });

            // set highlingt and tooltip on mouser over over the net
            linkWrap.on("mouseover", function (ev, d) {
                let netWrap = netToLink[getNet(d).id]["wrap"];
                d3__namespace.selectAll(netWrap)
                    .classed("link-wrap-activated", true);

                _this.tooltip.show(ev, getNameOfEdge(d));
            });
            linkWrap.on("mouseout", function (ev, d) {
                let netWrap = netToLink[getNet(d).id]["wrap"];
                d3__namespace.selectAll(netWrap)
                    .classed("link-wrap-activated", false);

                _this.tooltip.hide();
            });

            // set link highlight on net click
            function onLinkClick(ev, d) {
                let net = getNet(d);
                let doSelect = net.selected = !net.selected;
                // propagate click on all nets with same source

                let netCore = netToLink[net.id]["core"];
                d3__namespace.selectAll(netCore)
                    .classed("link-selected", doSelect);
                ev.stopPropagation();
            }

            // Select net on click
            link.on("click", onLinkClick);
            linkWrap.on("click", onLinkClick);
        }

        static fromYosys(yosysJson) {
            return yosys(yosysJson);
        }

        terminate() {
            if (this.layouter) {
                this.layouter.terminate();
            }
        }

        setErrorText(msg) {
            this.root.selectAll("*").remove();
            let errText = this.errorText;
            if (!errText) {
                errText = this.errorText = this.root.append("text")
                    .attr("x", "50%")
                    .attr("y", "50%")
                    .attr("dominant-baseline", "middle")
                    .attr("text-anchor", "middle")
                    .style("font-size", "34px");
            }
            errText.text(msg);
            let t = d3__namespace.zoomTransform(this.root.node());
            t.k = 1;
            t.x = 0;
            t.y = 0;
            this.root.attr("transform", t);

        }
    }

    exports.HwSchematic = HwSchematic;

    Object.defineProperty(exports, '__esModule', { value: true });

}));
