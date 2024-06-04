using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Engine.Platform;

namespace Engine.Utils;

public readonly struct XmlParserAttribute {
    public readonly string name;
    public readonly string value;

    internal XmlParserAttribute(XmlAttribute attribute) {
        this.name = attribute.Name;
        this.value = attribute.Value;
    }
}

public readonly struct XmlParserAttributes {

    private readonly XmlParserAttribute[] attributes;

    internal XmlParserAttributes(XmlAttributeCollection attributes) {
        this.attributes = new XmlParserAttribute[attributes == null ? 0 : attributes.Count];
        for (int i = 0 ; i < this.attributes.Length ; i++) {
            this.attributes[i] = new XmlParserAttribute(attributes[i]);
        }
    }

    public int Length {
        get => this.attributes.Length;
    }

    public XmlParserAttribute this[int index] { get => this.attributes[index]; }

    public IEnumerator<XmlParserAttribute> GetEnumerator() {
        return new XmlParserAttributesEnumerator(this.attributes);
    }

    private struct XmlParserAttributesEnumerator : IEnumerator<XmlParserAttribute> {
        private int index;
        private XmlParserAttribute[] attributes;

        public XmlParserAttributesEnumerator(XmlParserAttribute[] attributes) {
            this.index = -1;
            this.attributes = attributes;
        }

        public bool MoveNext() {
            index++;
            return index < this.attributes.Length;
        }

        public void Reset() {
            index = -1;
        }

        public void Dispose() {

        }


        public XmlParserAttribute Current => this.attributes[this.index];

        object IEnumerator.Current => this.attributes[this.index];

    }
}

public class XmlParserNodeList {
    private readonly XmlNode[] nodes;

    internal XmlParserNodeList(XmlNodeList list) {
        int length = 0;
        int count = list.Count;
        for (int i = 0 ; i < count ; i++) {
            switch (list[i].NodeType) {
                case XmlNodeType.Element:
                case XmlNodeType.Text:
                    length++;
                    break;
            }
        }

        this.nodes = new XmlNode[length];
        for (int i = 0, j = 0 ; i < count ; i++) {
            switch (list[i].NodeType) {
                case XmlNodeType.Element:
                case XmlNodeType.Text:
                    this.nodes[j++] = list[i];
                    break;
            }
        }
    }

    internal XmlParserNodeList(XmlNode[] nodes) {
        this.nodes = nodes;
    }

    public int Length { get => this.nodes.Length; }

    public XmlParserNode this[int index] { get => new XmlParserNode(this.nodes[index]); }

    public IEnumerator<XmlParserNode> GetEnumerator() {
        return new XmlParserNodeListEnumerator(this.nodes);
    }

    private struct XmlParserNodeListEnumerator : IEnumerator<XmlParserNode> {
        private int index;
        private XmlNode[] nodes;

        public XmlParserNodeListEnumerator(XmlNode[] nodes) {
            this.index = -1;
            this.nodes = nodes;
        }

        public bool MoveNext() {
            index++;
            return index < this.nodes.Length;
        }

        public void Reset() {
            index = -1;
        }

        void IDisposable.Dispose() {

        }

        object IEnumerator.Current => new XmlParserNode(this.nodes[this.index]);

        public XmlParserNode Current => new XmlParserNode(this.nodes[this.index]);
    }

}

public class XmlParserNode {

    private readonly XmlNode element;

    public readonly XmlParserAttributes Attributes;

    internal XmlParserNode(XmlNode elem) {
        this.element = elem;
        this.Attributes = new XmlParserAttributes(elem.Attributes);
    }

    public string OuterXML => this.element.OuterXml;

    public string GetAttribute(string name) {
        if (this.element is XmlElement) {
            XmlElement elem = this.element as XmlElement;
            return elem.HasAttribute(name) ? elem.GetAttribute(name) : null;
        }
        return null;
    }

    public string TagName => this.element.Name;

    public XmlParserNodeList Children => new XmlParserNodeList(this.element.ChildNodes);

    public int Length => this.element.ChildNodes.Count;

    public bool HasAttribute(string attributeName) {
        for (int i = 0 ; i < this.Attributes.Length ; i++) {
            if (this.Attributes[i].name == attributeName) return true;
        }
        return false;
    }

    public string TextContent {
        get
        {
            StringBuilder str = new StringBuilder(128);
            XmlNode sibling = this.element.FirstChild;
            int processed = 0;

            while (sibling != null) {
                if (sibling.NodeType == XmlNodeType.Text) {
                    processed++;
                    str.Append(sibling.Value);
                }
                sibling = sibling.NextSibling;
            }

            return processed < 1 ? null : str.ToString();
        }
    }

    public XmlParserNodeList GetChildrenList(string tag_name) {
        List<XmlNode> nodes = new List<XmlNode>();

        foreach (XmlNode node in this.element.ChildNodes) {
            if (node.NodeType == XmlNodeType.Element && node.Name == tag_name) {
                nodes.Add(node);
            }
        }

        return new XmlParserNodeList(nodes.ToArray());
    }

    public XmlParserNode GetChildren(string tag_name) {
        foreach (XmlNode node in this.element.ChildNodes) {
            if (node.NodeType == XmlNodeType.Element && node.Name == tag_name) {
                return new XmlParserNode(node);
            }
        }
        return null;
    }
}


public class XmlParser {

    private XmlDocument xml;

    private XmlParser() { }

    public static XmlParser Init(string src) {
        string text = FS.ReadText(src);
        try {
            XmlDocument doc = new XmlDocument();
            //doc.PreserveWhitespace = true;
            doc.LoadXml(text);

            return new XmlParser() { xml = doc };
        } catch (Exception e) {
            Logger.Error($"xmlparser_init() can not parse {src}:\n{e.Message}");
            return null;
        }
    }

    public void Destroy() {
        this.xml = null;
    }

    public XmlParserNode GetRoot() {
        return new XmlParserNode(this.xml.DocumentElement);
    }

}
