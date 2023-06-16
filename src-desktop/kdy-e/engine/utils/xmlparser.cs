using System;
using System.Text;
using System.Xml;
using Engine.Platform;

namespace Engine.Utils;

public class XmlParserAttribute {
    public readonly string name;
    public readonly string value;

    internal XmlParserAttribute(XmlAttribute attribute) {
        this.name = attribute.Name;
        this.value = attribute.Value;
    }
}

public class XmlParserAttributes {

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

    public System.Collections.IEnumerator GetEnumerator() {
        return new XmlParserAttributesEnumerator(this.attributes);
    }

    private class XmlParserAttributesEnumerator : System.Collections.IEnumerator {
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

        public object Current {
            get => this.attributes[this.index];
        }

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

    public System.Collections.IEnumerator GetEnumerator() {
        return new XmlParserNodeListEnumerator(this.nodes);
    }

    private class XmlParserNodeListEnumerator : System.Collections.IEnumerator {
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

        public object Current {
            get => new XmlParserNode(this.nodes[this.index]);
        }

    }


}

public class XmlParserNode {

    private XmlNode element;

    internal XmlParserNode(XmlNode elem) {
        this.element = elem;
    }

    public string OuterHTML { get => this.element.OuterXml; }

    public string GetAttribute(string name) {
        if (this.element is XmlElement) {
            XmlElement elem = this.element as XmlElement;
            return elem.HasAttribute(name) ? elem.GetAttribute(name) : null;
        }
        return null;
    }

    public string TagName { get => this.element.Name; }

    public XmlParserNodeList Children {
        get => new XmlParserNodeList(this.element.ChildNodes);
        /*public XmlParserNodeList Children {
            get
            {
                XmlNodeList children = this.element.ChildNodes;
                for (int i = 0 ; i < children.Count ; i++) {
                    yield return new XmlParserNode(children[i]);
                }
            }
        }*/
    }

    public int Length { get => this.element.ChildNodes.Count; }

    public bool HasAttribute(string attributeName) {
        if (this.element is XmlElement)
            return ((XmlElement)this.element).HasAttribute(attributeName);
        else
            return false;
    }

    public XmlParserAttributes Attributes {
        get => new XmlParserAttributes(element.Attributes);
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
        System.Collections.Generic.List<XmlNode> nodes = new System.Collections.Generic.List<XmlNode>();

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
            Console.Error.WriteLine("xmlparser_init() can not parse " + src + ":\n" + e.Message);
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
