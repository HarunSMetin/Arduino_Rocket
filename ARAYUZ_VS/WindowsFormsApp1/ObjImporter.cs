using OpenTK;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TOBBETUROCKETRY
{
    class ObjImporter
    {

        public static void LoadModel(string filePath, out List<Vector3> vertices, out List<int> triangles, out List<Vector3> normals, out List<Vector2> texCoords)
        {
            vertices = new List<Vector3>();
            triangles = new List<int>();
            normals = new List<Vector3>();
            texCoords = new List<Vector2>();

            if (File.Exists(filePath))
            {

                string[] lines = File.ReadAllLines(filePath);

                for (int i = 0; i < lines.Length; i++)
                {
                    if (lines[i].Contains("v "))

                    {
                        string[] tokens = lines[i].Split(' ');

                        float x = 0, y = 0, z = 0;

                        float.TryParse(tokens[1], out x);
                        float.TryParse(tokens[2], out y);

                        if (tokens.Length > 3)
                            float.TryParse(tokens[3], out z);

                        vertices.Add(new Vector3(x, y, z));
                    }
                    else if (lines[i].Contains("f "))
                    {
                        string[] tokens = lines[i].Split(' ');
                        for (int j = 1; j < tokens.Length; j++)
                        {
                            if (tokens[j].Contains('/'))
                            {
                                string[] subTokens = tokens[j].Split('/');

                                int index;
                                for(int a =0; a < subTokens.Length;a++)
                                {
                                    if(Int32.TryParse(subTokens[0], out index))
                                    {
                                        triangles.Add(index - 1);
                                    }
                                    
                                }
                                
                            }
                            else
                            {
                                int index;
                                if (Int32.TryParse(tokens[j], out index))
                                {
                                    triangles.Add(index - 1);
                                }

                            }

                        }

                        //for (int j = 1; j < tokens.Length; j += 2)
                        //{
                        //    int index = 0;
                        //    Int32.TryParse(tokens[j], out index);
                        //
                        //    triangles.Add(index - 1);
                        //}
                    }
                    else if (lines[i].Contains("vt "))
                    {
                        string[] tokens = lines[i].Split(' ');

                        float x = 0, y = 0;

                        float.TryParse(tokens[1], out x);
                        float.TryParse(tokens[2], out y);

                        texCoords.Add(new Vector2(x, y));
                    }
                    else if (lines[i].Contains("vn "))
                    {
                        string[] tokens = lines[i].Split(' ');

                        float x = 0, y = 0, z = 0;

                        float.TryParse(tokens[1], out x);
                        float.TryParse(tokens[2], out y);
                        float.TryParse(tokens[3], out z);

                        normals.Add(new Vector3(x, y, z));
                    }

                }

            }
            else
            {
                throw new System.Exception("OBJ file not found: " + filePath);
            }
        }

    }

    class ObjParser
    {
        static private List<Vector3> vertices = new List<Vector3>();
        static private List<Vector3> normals = new List<Vector3>();
        static private List<Vector2> texCoords = new List<Vector2>();
        static private List<int[]> faces = new List<int[]>();

        static public void ParseFile(string filePath)
        {
            using (StreamReader reader = new StreamReader(filePath))
            {
                string line;
                while ((line = reader.ReadLine()) != null)
                {
                    string[] tokens = line.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

                    if (tokens.Length == 0)
                        continue;

                    switch (tokens[0])
                    {
                        case "v":
                            float x = float.Parse(tokens[1]);
                            float y = float.Parse(tokens[2]);
                            float z = float.Parse(tokens[3]);
                            vertices.Add(new Vector3(x, y, z));
                            break;

                        case "vn":
                            float nx = float.Parse(tokens[1]);
                            float ny = float.Parse(tokens[2]);
                            float nz = float.Parse(tokens[3]);
                            normals.Add(new Vector3(nx, ny, nz));
                            break;

                        case "vt":
                            float u = float.Parse(tokens[1]);
                            float v = float.Parse(tokens[2]);
                            texCoords.Add(new Vector2(u, v));
                            break;

                        case "f":
                            int[] face = new int[9];
                            for (int i = 0; i < 3; i++)
                            {
                                string[] vertexTokens = tokens[i + 1].Split(new char[] { '/' }, StringSplitOptions.None);
                                face[i * 3] = int.Parse(vertexTokens[0]) - 1; // vertex index
                                face[i * 3 + 1] = int.Parse(vertexTokens[1]) - 1; // texture coordinate index
                                face[i * 3 + 2] = int.Parse(vertexTokens[2]) - 1; // normal index
                            }
                            faces.Add(face);
                            break;
                    }
                }
            }
        }

        public static List<Vector3> Vertices { get { return vertices; } }
        public static List<Vector3> Normals { get { return normals; } }
        public static List<Vector2> TexCoords { get { return texCoords; } }
        public static List<int[]> Faces { get { return faces; } }
    }
}

